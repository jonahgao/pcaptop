#include "syn_stat.h"

#include <algorithm>
#include "net_types.h"

SynStat::SynStat(int district, int precision):
    district_(district),
    precision_(precision)
{ }

SynStat::~SynStat() {
}

void SynStat::addData(const DataPoint& dp, time_t tm) {
    bool is_syn = dp.flags & TH_SYN;
    if (!is_syn) 
        return;

    int dist = tm / precision_;

    LockGuard guard(mu_);

    if (datas_.empty() || datas_.back().dist != dist)  {
        datas_.push_back(Elem());
        datas_.back().dist = dist;

        removeStaleUnlock(dist);
    }

    SynCntMap& m = datas_.back().m;
    ++m[dp.ip];
}

void SynStat::removeStaleUnlock(int dist) {
    while (!datas_.empty() && datas_.front().dist <= dist - district_ / precision_) {
        datas_.pop_front();
    }
}

static bool synGreaterComp(const SynStat::Result& lh, const SynStat::Result& rh) {
    return lh.nums_of_syn > rh.nums_of_syn;
}

void SynStat::getResults(int count, std::vector<Result>& vec) {
    SynCntMap aggre;

    {
        LockGuard guard(mu_);

        removeStaleUnlock(getCurrentSeconds() / precision_);

        for (std::deque<Elem>::iterator it = datas_.begin(); it != datas_.end(); ++it) {
            SynCntMap& m = it->m;
            for (SynCntMap::iterator it2 = m.begin(); it2 != m.end(); ++it2) 
                aggre[it2->first] += it2->second;
        }
    }

    if (aggre.empty()) return;

    vec.reserve(aggre.size());
    for (SynCntMap::iterator it = aggre.begin(); it != aggre.end(); ++it) {
        Result r;
        r.ip = it->first;
        r.nums_of_syn = it->second;
        vec.push_back(r);
    }

    int num = std::min(aggre.size(), static_cast<size_t>(count));
    std::partial_sort(vec.begin(), vec.begin() + num, vec.end(), synGreaterComp);
    vec.resize(num);
}

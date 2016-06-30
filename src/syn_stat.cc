#include "syn_stat.h"

#include <algorithm>
#include "net_types.h"

SynStat::SynStat(int district, int precision):
    district_(district),
    precision_(precision),
    capacity_(district / precision),
    last_time_(0)
{ }

SynStat::~SynStat() {
}

void SynStat::addData(const DataPoint& dp, time_t t) {
    bool is_syn = dp.flags & TH_SYN;
    if (!is_syn) 
        return;

    t = t / precision_;

    LockGuard guard(mu_);

    if (datas_.empty() || last_time_ != t)  {
        datas_.push_back(SynCntMap());
        last_time_ = t;

        while (datas_.size() > capacity_) 
            datas_.pop_front();
    }

    SynCntMap& m = datas_.back();
    ++m[dp.ip];
}

static bool synGreaterComp(const SynStat::Result& lh, const SynStat::Result& rh) {
    return lh.nums_of_syn > rh.nums_of_syn;
}

void SynStat::getResults(int count, std::vector<Result>& vec) {
    SynCntMap aggre;

    {
        LockGuard guard(mu_);

        for (std::deque<SynCntMap>::iterator it = datas_.begin(); it != datas_.end(); ++it) {
            SynCntMap& m = *it;
            for (SynCntMap::iterator it2 = m.begin(); it2 != m.end(); ++it2) 
                aggre[it2->first] += it2->second;
        }
    }

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

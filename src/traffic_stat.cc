#include "traffic_stat.h"

#include <limits>
#include <algorithm>
#include "net_types.h"

TrafficStat::TrafficStat(int district, int precision) :
    district_(district),
    precision_(precision)
{
}


TrafficStat::~TrafficStat() {
}

void TrafficStat::addFlow(const DataPoint& dp, TrafficCount& v) {
    if (dp.direc == IN)
        v.in += dp.pktlen;
    else 
        v.out += dp.pktlen;
}

void TrafficStat::removeStaleUnlock(int dist) {
    while (!datas_.empty() && datas_.front().dist <= dist - district_ / precision_) {
        datas_.pop_front();
    }
}

void TrafficStat::addData(const DataPoint& dp, time_t tm) {
    int dist = tm / precision_;

    LockGuard guard(mu_);

    if (datas_.empty() || datas_.back().dist != dist) {
        datas_.push_back(Elem());
        datas_.back().dist = dist;

        removeStaleUnlock(dist);
    }

    FlowCntMap& m = datas_.back().m;

    FlowCntMap::iterator last = m.end(); // ip:port相同idx最大的那个
    SrcAddr addr(dp.ip, dp.port, std::numeric_limits<int>::max());
    FlowCntMap::iterator it = m.upper_bound(addr);
    if (it != m.begin()) {
        --it;
        if (it->first.ip == dp.ip && last->first.port == dp.port)
            last = it;
    }

    bool is_syn = dp.flags & TH_SYN;
    if (is_syn) {
        SrcAddr sa(dp.ip, dp.port);
        if (last != m.end())
            sa.idx = last->first.idx + 1;

        addFlow(dp, m[sa]);
    }
    else {
        if (last != m.end()) {
            addFlow(dp, last->second);
        }
        else {
            SrcAddr sa(dp.ip, dp.port);
            addFlow(dp, m[sa]);
        }
    }
}

struct FlowGreaterComp {
    TrafficStat::SortType t;

    FlowGreaterComp(TrafficStat::SortType _t) : t(_t) {
    }

    bool operator()(const TrafficStat::Result& lh, const TrafficStat::Result& rh) {
        switch (t) {
            case TrafficStat::SORT_BY_IN:
                return lh.flow.in > rh.flow.in;
                break;
            case TrafficStat::SORT_BY_OUT:
                return lh.flow.out > rh.flow.out;
                break;
            default:
                return (lh.flow.in + lh.flow.out) > (rh.flow.in + rh.flow.out);
        }
    }
};


void TrafficStat::getResults(int count, SortType type, std::vector<Result>& vec) {
    FlowCntMap aggre;

    {
        LockGuard guard(mu_);
        
        removeStaleUnlock(getCurrentSeconds() / precision_);

        for (std::deque<Elem>::iterator it = datas_.begin(); it!= datas_.end(); ++it) {
            FlowCntMap &m = it->m;
            for (FlowCntMap::iterator iter = m.begin(); iter != m.end(); ++iter) {
                aggre[iter->first].in += iter->second.in;
                aggre[iter->first].out += iter->second.out;
            }
        }
    }

    vec.reserve(aggre.size());
    for (FlowCntMap::iterator it = aggre.begin(); it != aggre.end(); ++it) {
        vec.push_back(Result());
        vec.back().addr = it->first;
        vec.back().flow = it->second;
    }
    int num = std::min(aggre.size(), static_cast<size_t>(count));
    std::partial_sort(vec.begin(), vec.begin() + num, vec.end(), FlowGreaterComp(type));
    vec.resize(num);
}

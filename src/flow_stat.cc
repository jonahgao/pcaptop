#include "flow_stat.h"

#include <limits>
#include "net_types.h"

FlowStat::FlowStat(int district, int precision) :
    district_(district),
    precision_(precision),
    capacity_(district / precision)
{
}


FlowStat::~FlowStat() {
}

void FlowStat::addFlow(const DataPoint& dp, FlowCount& v) {
    if (dp.direc == IN)
        v.in += dp.pktlen;
    else 
        v.out += dp.pktlen;
}

void FlowStat::addData(const DataPoint& dp, time_t t) {
    t = t / precision_;

    LockGuard guard(mu_);

    if (datas_.empty() || datas_.back().t != t) {
        datas_.push_back(Elem());
        datas_.back().t = t;

        while (datas_.size() > capacity_) 
            datas_.pop_front();
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

void FlowStat::getResults(int count, std::vector<Result>& vec) {
    FlowCntMap aggre;

    {
        LockGuard guard(mu_);
    }
    
}

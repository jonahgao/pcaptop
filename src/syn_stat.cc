#include "syn_stat.h"
#include "net_types.h"

SynStat::SynStat(int district, int precision):
    district_(district),
    precision_(precision),
    capacity_(district / precision)
{ }

SynStat::~SynStat() {
}

void SynStat::addData(const DataPoint& dp, time_t t) {
    bool is_syn = dp.flags & TH_SYN;
    if (!is_syn) 
        return;

    t = t / precision_;

    if (datas_.empty() || datas_.back().t != t)  {
        datas_.push(Elem());
        datas_.back().t = t;

        while (datas_.size() > capacity_) 
            datas_.pop();
    }

    Map& m = datas_.back().m;
    ++m[dp.ip];
}

void SynStat::getResults(int count, std::vector<Result>& vec) {
    //TODO:
}

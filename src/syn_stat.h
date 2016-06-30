#ifndef SYN_STAT_H_9BMC37ZE
#define SYN_STAT_H_9BMC37ZE
#include <vector>
#include <map>
#include <deque>
#include <vector>
#include "mutex.h"
#include "common.h"

class SynStat {
public:
    struct Result {
        std::string ip;
        uint64_t nums_of_syn;

        Result() : nums_of_syn(0) {}
    };

public:
    SynStat(int district, int precision);
    ~SynStat();

    void addData(const DataPoint& dp, time_t t);

    void getResults(int count, std::vector<Result>& vec);

private:
    typedef std::map<std::string, uint64_t> SynCntMap;

private:
    const int district_;
    const int precision_;
    std::deque<SynCntMap> datas_;
    const int capacity_;
    time_t last_time_;
    Mutex mu_;
};



#endif /* end of include guard: SYN_STAT_H_9BMC37ZE */

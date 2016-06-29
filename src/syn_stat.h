#ifndef SYN_STAT_H_9BMC37ZE
#define SYN_STAT_H_9BMC37ZE
#include <vector>
#include <map>
#include <queue>
#include "common.h"

class SynStat {
public:
    struct Result {
        std::string ip;
        uint64_t nums_of_syn;
    };

public:
    SynStat(int district, int precision);
    ~SynStat();

    void addData(const DataPoint& dp, time_t t);

    void getResults(int count, std::vector<Result>& vec);

private:
    typedef std::map<std::string, uint64_t> Map;

    struct Elem {
        time_t t;
        Map m;
    };

private:
    const int district_;
    const int precision_;
    std::queue<Elem> datas_;
    const int capacity_;
};



#endif /* end of include guard: SYN_STAT_H_9BMC37ZE */

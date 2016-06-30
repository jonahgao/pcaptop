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

    void addData(const DataPoint& dp, time_t tm);

    void getResults(int count, std::vector<Result>& vec);

private:
    typedef std::map<std::string, uint64_t> SynCntMap;

    struct Elem {
        int dist;       // 时间区间
        SynCntMap m;
    };

private:
    void removeStaleUnlock(int dist);

private:
    const int district_;
    const int precision_;
    std::deque<Elem> datas_;
    Mutex mu_;
};



#endif /* end of include guard: SYN_STAT_H_9BMC37ZE */

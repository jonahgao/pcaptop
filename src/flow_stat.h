#ifndef FLOW_STAT_H_XVAT5DKN
#define FLOW_STAT_H_XVAT5DKN
#include <time.h>
#include <deque>
#include <map>
#include <vector>
#include "common.h"
#include "mutex.h"

class FlowStat {
public:
    struct SrcAddr {
        std::string ip;
        uint16_t port;
        int idx;

        SrcAddr() : port(0), idx(1) {}

        SrcAddr(const std::string& ip_,
                uint16_t port_,
                int idx_ = 1) :
            ip(ip_), port(port_), idx(idx_) { }

        bool operator==(const SrcAddr& rhs) const {
            return ip == rhs.ip && port == rhs.port && idx == rhs.idx;
        }

        bool operator<(const SrcAddr& rhs) const {
            return (ip == rhs.ip) ? (port == rhs.port ? idx < rhs.idx : port < rhs.port) : (ip < rhs.ip);
        }
    };

    struct FlowCount {
        uint64_t in;
        uint64_t out;

        FlowCount() : in(0), out(0) {}
    };
    
    // 流量按什么排序
    enum SortType {
        SORT_BY_IN,
        SORT_BY_OUT,
        SORT_BY_TOTOAL
    };

    struct Result {
        SrcAddr addr;
        FlowCount flow;
    };

public:
    /**
     * district: 统计区间长度，单位秒
     * precision: 精度，单位秒
     */
    FlowStat(int district, int precision);

    ~FlowStat();

    void addData(const DataPoint& dp, time_t t);

    void getResults(int count, SortType type, std::vector<Result>& vec);

private:
    typedef std::map<SrcAddr, FlowCount> FlowCntMap;

    struct Elem {
        int dist;
        FlowCntMap m;
    };

private:
    void addFlow(const DataPoint& dp, FlowCount& v);
    void removeStaleUnlock(int dist);
    
private:
    const int district_;
    const int precision_;
    std::deque<Elem> datas_;
    Mutex mu_;
};



#endif /* end of include guard: FLOW_STAT_H_XVAT5DKN */

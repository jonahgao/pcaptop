#ifndef TRAFFIC_STAT_H_CEAIF3MU
#define TRAFFIC_STAT_H_CEAIF3MU

#include <time.h>
#include <deque>
#include <map>
#include <vector>
#include "common.h"
#include "mutex.h"

class TrafficStat {
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

    struct TrafficCount {
        uint64_t in;
        uint64_t out;

        TrafficCount() : in(0), out(0) {}
    };
    
    // 流量按什么排序
    enum SortType {
        SORT_BY_IN,
        SORT_BY_OUT,
        SORT_BY_TOTOAL
    };

    struct Result {
        SrcAddr addr;
        TrafficCount flow;
    };

public:
    /**
     * district: 统计区间长度，单位秒
     * precision: 精度，单位秒
     */
    TrafficStat(int district, int precision);

    ~TrafficStat();

    void addData(const DataPoint& dp, time_t t);

    void getResults(int count, SortType type, std::vector<Result>& vec);

private:
    typedef std::map<SrcAddr, TrafficCount> FlowCntMap;

    struct Elem {
        int dist;
        FlowCntMap m;
    };

private:
    void addFlow(const DataPoint& dp, TrafficCount& v);
    void removeStaleUnlock(int dist);
    
private:
    const int district_;
    const int precision_;
    std::deque<Elem> datas_;
    Mutex mu_;
};


#endif /* end of include guard: TRAFFIC_STAT_H_CEAIF3MU */

#ifndef FLOW_STAT_H_XVAT5DKN
#define FLOW_STAT_H_XVAT5DKN
#include <time.h>
#include <queue>
#include <map>
#include "common.h"

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

    struct FlowRate {
        uint64_t in;
        uint64_t out;

        FlowRate() : in(0), out(0) {}
    };

    struct Result {
        SrcAddr addr;
        FlowRate flow;
    };

public:
    /**
     * district: 统计区间，单位秒
     * precision: 精度，单位秒
     */
    FlowStat(int district, int precision);

    ~FlowStat();

    void addData(const DataPoint& dp, time_t t);

    void getResults(int count, std::vector<Result>& vec);

private:
    typedef std::map<SrcAddr, FlowRate> Map;

    struct Elem {
        time_t t;
        Map m;
    };

private:
    void addFlowRate(const DataPoint& dp, FlowRate& v);
    
private:
    const int district_;
    const int precision_;
    std::queue<Elem> datas_;
    const int capacity_;
};



#endif /* end of include guard: FLOW_STAT_H_XVAT5DKN */

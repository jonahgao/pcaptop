#include "ui.h"

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <map>
#include <queue>
#include <limits>
#include <ncurses.h>
#include "net_types.h"

using namespace std;

struct SrcAddr {
    std::string ip;
    uint16_t port;
    int idx;

    SrcAddr() : port(0), idx(1) {}
    
    SrcAddr(const std::string& ip_,
            uint16_t port_,
            int idx_ = 1) :
        ip(ip_), port(port_), idx(idx_) { }
};

struct FlowValue {
    uint64_t in;
    uint64_t out;

    FlowValue() : in(0), out(0) {}
};

static bool operator==(const SrcAddr& lhs, const SrcAddr& rhs) {
    return lhs.ip == rhs.ip &&
        lhs.port == rhs.port &&
        lhs.idx == rhs.idx;
}

static bool operator<(const SrcAddr& lhs, const SrcAddr& rhs) {
    return (lhs.ip == rhs.ip) ? (lhs.port == rhs.port ? lhs.idx < rhs.idx : lhs.port < rhs.port)
                              : (lhs.ip < rhs.ip);
}

typedef std::map<std::string, uint64_t> SynCntMap;   // key为ip， value为syn次数
typedef std::map<SrcAddr, FlowValue> FlowCntMap;     // 

struct SynStatElem {
    time_t t;
    SynCntMap m;
};

struct FlowStatElem {
    time_t t;
    FlowCntMap m;
};

std::queue<SynStatElem> syn_stats;          // 存储syn次数数据，1秒一个队列元素, 队列最大长度约为600个
std::queue<FlowStatElem> flow_stats;        // 存储流量统计数据，1秒一个队列元素, 队列最大长度约为600个
pthread_mutex_t mutex;

static const int kMaxQueueSize = 610;        // 只存储约10分钟的数据

void initUI(const char* interface) {
    // init thread
    int ret = pthread_mutex_init(&mutex, NULL);
    if (ret != 0) {
        perror("call pthread_mutex_init failed");
        exit(-1);
    }

    // init ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}

void exitUI() {
    // end ncurses mode
    endwin();

    pthread_mutex_destroy(&mutex);
}

static void addFlowValue(const DataPoint& dp, FlowValue& v) {
    if (dp.direc == IN)
        v.in += dp.pktlen;
    else 
        v.out += dp.pktlen;
}

static time_t getCurrentSeconds() {
    struct timespec ts;
    int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (ret != 0) {
        perror("call clock_gettime failed");
        exitUI();
        exit(-1);
    }
    return ts.tv_sec;
}

void addData(const DataPoint& dp) {
    //std::cout << dp << std::endl;
    time_t now = getCurrentSeconds();

    bool is_syn = dp.flags & TH_SYN;
    SrcAddr addr(dp.ip, dp.port, std::numeric_limits<int>::max());

    int ret = pthread_mutex_lock(&mutex);
    if (ret != 0) {
        perror("call pthread_mutex_lock failed");
        exitUI();
        exit(-1);
    }

    // 处理syn统计数据
    if (is_syn) {
        if (syn_stats.empty() || syn_stats.back().t != now)  {
            syn_stats.push(SynStatElem());
            syn_stats.back().t = now;
        }

        assert(!syn_stats.empty());
        SynCntMap& m = syn_stats.back().m;
        ++m[dp.ip];

        while (syn_stats.size() >= kMaxQueueSize) 
            syn_stats.pop();
    }

    // 处理流量统计数据
    if (flow_stats.empty() || flow_stats.back().t != now) {
        flow_stats.push(FlowStatElem());
        flow_stats.back().t = now;

        while (flow_stats.size() >= kMaxQueueSize)
            flow_stats.pop();
    }

    assert(!flow_stats.empty());
    FlowCntMap& fm = flow_stats.back().m;

    FlowCntMap::iterator last = fm.end(); // ip:port相同idx最大的那个
    FlowCntMap::iterator it = fm.upper_bound(addr);
    if (it != fm.begin()) {
        --it;
        if (it->first.ip == dp.ip && last->first.port == dp.port)
            last = it;
    }

    if (is_syn) {
        SrcAddr sa(dp.ip, dp.port);
        if (last != fm.end())
            sa.idx = last->first.idx + 1;

        addFlowValue(dp, fm[sa]);
    }
    else {
        if (last != fm.end()) {
            addFlowValue(dp, last->second);
        }
        else {
            SrcAddr sa(dp.ip, dp.port);
            addFlowValue(dp, fm[sa]);
        }
    }

    ret = pthread_mutex_unlock(&mutex);
    if (ret != 0) {
        perror("call pthread_mutex_unlock failed");
        exitUI();
        exit(-1);
    }
}

#include "ui.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <iostream>
#include "net_types.h"
#include "traffic_stat.h"
#include "syn_stat.h"

using namespace std;

// 1秒刷新1次
static const int kRefreshIntervalSec = 1;

// 统计区间长度
enum DistrictLength {
    kOneMinutes = 0,    // 显示最近1分钟之内的数据
    kFiveMinutes,   // 显示最近5分钟之内的数据
    kTenMinutes,    // 显示最近10分钟之内的数据
    kTestMinutes,   // 测试
};

static const char* kDistrictLengthName[] =  {
    "1 minutes",
    "5 minutes",
    "10 minutes",
    "40 seconds",
};

TrafficStat f1(60, 2);    // 1分钟
TrafficStat f5(300, 10);   // 5分钟
TrafficStat f10(600, 20);   // 10分钟
TrafficStat ft(40, 1);

SynStat s1(60, 2);
SynStat s5(300, 10);
SynStat s10(600, 20);

static pthread_t ui_thread;

void initUI(const char* interface) {
    // init ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    halfdelay(kRefreshIntervalSec * 10);

    // init ui update thread
    void *updateUIRoutine(void *arg);
    int ret = pthread_create(&ui_thread, NULL, updateUIRoutine, NULL);
    if (ret != 0) {
        perror("call pthread_create failed");
        endwin();
        exit(-1);
    }
}

void exitUI() {
    // end ncurses mode
    endwin();
}

void addData(const DataPoint& dp) {
    //std::cout << dp << std::endl;
    time_t now = getCurrentSeconds();

    f1.addData(dp, now);
    f5.addData(dp, now);
    f10.addData(dp, now);
    ft.addData(dp, now);

    s1.addData(dp, now);
    s5.addData(dp, now);
    s10.addData(dp, now);
}

void printSyn(int starty, const std::vector<SynStat::Result>& results) {
    static const int kSpace = 3;
    static const int kAddrLen = strlen("255.255.255.255") + kSpace;
    
    // 状态烂
    mvchgat(starty, 0 , kAddrLen + 4, A_REVERSE, 1, NULL);    
    attron(A_REVERSE);
    mvprintw(starty, 0, "CLIENT");
    mvprintw(starty, kAddrLen, "SYN");
    attroff(A_REVERSE);

    for (size_t i = 0; i < results.size(); ++i) {
        const SynStat::Result& r = results[i];

        mvprintw(starty + 1 + i, 0, "%s", r.ip.c_str());
        mvprintw(starty + 1 + i, kAddrLen, "%lu", r.nums_of_syn);
    }
}

void printTraffic(int starty, const std::vector<TrafficStat::Result>& results) {
    static const int kSpace = 3;
    static const int kAddrLen = strlen("255.255.255.255:65535_65535") + kSpace;
    static const int kTrafficLen = strlen("99999.999GB") + kSpace;

    // 状态烂
    mvchgat(starty, 0 , kAddrLen + kTrafficLen * 3 + 4, A_REVERSE, 1, NULL);    
    attron(A_REVERSE);
    mvprintw(starty, 0, "CLIENT");
    mvprintw(starty, kAddrLen, "IN");
    mvprintw(starty, kAddrLen + kTrafficLen, "OUT");
    mvprintw(starty, kAddrLen + kTrafficLen * 2, "TOTAL");
    attroff(A_REVERSE);

    for (size_t i = 0; i < results.size(); ++i) {
        const TrafficStat::Result& r = results[i];

        // 源地址
        char idx[15] = {'\0'};
        if (r.addr.idx != 1)
            snprintf(idx, 15, "_%d", r.addr.idx);
        mvprintw(starty + 1 + i, 0, "%s:%u%s", r.addr.ip.c_str(), r.addr.port, idx);

        // IN
        mvprintw(starty + 1 + i, kAddrLen, "%s", perfectFlowValue(r.traff.in).c_str());
        mvprintw(starty + 1 + i, kAddrLen + kTrafficLen, "%s", perfectFlowValue(r.traff.out).c_str());
        mvprintw(starty + 1 + i, kAddrLen + kTrafficLen * 2, "%s", 
                perfectFlowValue(r.traff.in + r.traff.out).c_str());
    }
}

void refreshUI(DistrictLength dl) {
    erase();

    // 打印流量统计提示
    const char *traff_hint = "Network traffic in past ";
    mvprintw(0, 0, traff_hint);
    attron(A_BOLD);
    mvprintw(0, strlen(traff_hint), kDistrictLengthName[dl]);
    attroff(A_BOLD);

    int traff_cnt = 20;
    int active = 0;
    std::vector<TrafficStat::Result> traffic_results;
    switch (dl) {
    case kOneMinutes:
        f1.getResults(traff_cnt, TrafficStat::SORT_BY_TOTOAL, traffic_results, active);
        break;
    case kFiveMinutes:
        f5.getResults(traff_cnt, TrafficStat::SORT_BY_TOTOAL, traffic_results, active);
        break;
    case kTenMinutes:
        f10.getResults(traff_cnt, TrafficStat::SORT_BY_TOTOAL, traffic_results, active);
        break;
    case kTestMinutes:
        ft.getResults(traff_cnt, TrafficStat::SORT_BY_TOTOAL, traffic_results, active);
        break;
    }
    printw(": (%d active)", active);
    printTraffic(2, traffic_results);

    int syn_cnt = 15;
    std::vector<SynStat::Result> syn_results;
    switch (dl) {
    case kOneMinutes:
        s1.getResults(syn_cnt, syn_results);
        break;
    case kFiveMinutes:
        s5.getResults(syn_cnt, syn_results);
        break;
    case kTenMinutes:
        s10.getResults(syn_cnt, syn_results);
        break;
    case kTestMinutes:
        s1.getResults(syn_cnt, syn_results);
        break;
    }
    
    printSyn(1 + traffic_results.size() + 4, syn_results);

    refresh();
};

void *updateUIRoutine(void *arg) {
    bool should_refresh = false;
    time_t last_refresh = 0;
    DistrictLength dl = kOneMinutes;
    char ch;

    refreshUI(dl);

    while (true) {
        should_refresh = false;

        ch = getch();

        switch (ch) {
            case 'o':
                should_refresh = true;
                dl = kOneMinutes;
                break;
            case 'f':
                should_refresh = true;
                dl = kFiveMinutes;
                break;
            case 't':
                should_refresh = true;
                dl = kTenMinutes;
                break;
            case 'x':
                should_refresh = true;
                dl = kTestMinutes;
                break;
            case ERR:           //超时
                should_refresh = true;
                break;
            default:
                should_refresh = getCurrentSeconds() 
                    - last_refresh >= kRefreshIntervalSec;
                break;
        }

        if (should_refresh) {
            last_refresh = getCurrentSeconds();
            refreshUI(dl);
        }
    }

    return NULL;
}

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

void printTraffic(const std::vector<TrafficStat::Result>& results, int lines) {
    if (lines < 2) 
        return;
    
    // 状态烂
    attron(A_REVERSE);
    mvprintw(1, 0, "CLIENT\tIN\tOUT\tTOTAL                                ");
    attroff(A_REVERSE);

    for (size_t i = 0; i < results.size(); ++i) {
        const TrafficStat::Result& r = results[i];
        mvprintw(i + 2, 0, "%s:%u\t%s\t%s\t%s", r.addr.ip.c_str(), r.addr.port, 
                perfectFlowValue(r.flow.in).c_str(),
                perfectFlowValue(r.flow.out).c_str(),
                perfectFlowValue(r.flow.in + r.flow.out).c_str());
    }
}

void printSyn(const std::vector<SynStat::Result>& results, int lines) {

}

void refreshUI(DistrictLength dl) {
    erase();

    // 打印流量统计提示
    const char *traff_hint = "Network traffic in past ";
    mvprintw(0, 0, traff_hint);
    attron(A_BOLD);
    mvprintw(0, strlen(traff_hint), kDistrictLengthName[dl]);
    attroff(A_BOLD);
    addstr(": ");

    std::vector<TrafficStat::Result> traffic_results;
    ft.getResults(20, TrafficStat::SORT_BY_TOTOAL, traffic_results);
    printTraffic(traffic_results, 20 + 1);

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
            //TODO:
            //refreshUI(kTestMinutes);
            refreshUI(dl);
        }
    }

    return NULL;
}

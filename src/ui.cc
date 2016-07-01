#include "ui.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <iostream>
#include "net_types.h"
#include "syn_stat.h"
#include "flow_stat.h"

using namespace std;

// 1秒刷新1次
static const int kRefreshIntervalSec = 1;

enum ShowMode {
    kOneMinutes,    // 显示最近1分钟之内的数据
    kFiveMinutes,   // 显示最近5分钟之内的数据
    kTenMinutes,    // 显示最近10分钟之内的数据
    kTestMinutes,   // 测试
};

FlowStat f1(60, 2);    // 1分钟
FlowStat f5(300, 10);   // 5分钟
FlowStat f10(600, 20);   // 10分钟

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

    s1.addData(dp, now);
    s5.addData(dp, now);
    s10.addData(dp, now);
}


void refreshUI(ShowMode mode) {
    erase();

    if (mode == kOneMinutes) {
        mvprintw(0, 0, "One");
    }
    else if (mode == kFiveMinutes) {
        mvprintw(0, 0, "Five");
    }
    else if (mode == kTenMinutes) {
        mvprintw(0, 0, "Ten");
    }
    else {
    }
        //std::vector<SynStat::Result> syn_results;
        //s1.getResults(10, syn_results);
        ////for (int i = 0; i < syn_results.size(); ++i) {
            ////cout << syn_results[i].ip << "\t" << syn_results[i].nums_of_syn << endl;
        ////}
        ////

        //std::vector<FlowStat::Result> flow_results;
        //f1.getResults(10,  FlowStat::SORT_BY_TOTOAL, flow_results);
        //for (size_t i = 0; i < flow_results.size(); ++i) {
            //FlowStat::Result& r = flow_results[i];
            //mvprintw(i, 0, "%s:%u\t%s", r.addr.ip.c_str(), r.addr.port, 
                    //perfectFlowValue(r.flow.in + r.flow.out).c_str());
            ////std::cout << r.addr.ip << ":" << r.addr.port << "\t" << perfectFlowValue(r.flow.in + r.flow.out) << std::endl;
        //}

    refresh();
};

void *updateUIRoutine(void *arg) {
    bool should_refresh = false;
    time_t last_refresh = 0;
    ShowMode mode = kOneMinutes;
    char ch;

    refreshUI(mode);

    while (true) {
        should_refresh = false;

        ch = getch();

        switch (ch) {
            case 'o':
                should_refresh = true;
                mode = kOneMinutes;
                break;
            case 'f':
                should_refresh = true;
                mode = kFiveMinutes;
                break;
            case 't':
                should_refresh = true;
                mode = kTenMinutes;
                break;
            case ERR:
                should_refresh = true;
                break;
            default:
                should_refresh = getCurrentSeconds() 
                    - last_refresh >= kRefreshIntervalSec;
                break;
        }

        if (should_refresh) {
            last_refresh = getCurrentSeconds();
            refreshUI(mode);
        }
    }
}

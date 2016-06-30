#include "ui.h"

#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include "net_types.h"
#include "syn_stat.h"
#include "flow_stat.h"

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

    f1.addData(dp, now);
    f5.addData(dp, now);
    f10.addData(dp, now);

    s1.addData(dp, now);
    s5.addData(dp, now);
    s10.addData(dp, now);
}

void *updateUIRoutine(void *arg) {
    while (true) {
        sleep(1);

    }
}



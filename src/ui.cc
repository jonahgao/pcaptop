#include "ui.h"

#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include "net_types.h"
#include "syn_stat.h"
#include "flow_stat.h"

pthread_mutex_t mutex;

FlowStat f1(60, 2);    // 1分钟
FlowStat f5(300, 10);   // 5分钟
FlowStat f10(600, 20);   // 10分钟

SynStat s1(60, 2);
SynStat s5(300, 10);
SynStat s10(600, 20);

static pthread_t ui_thread;

void initUI(const char* interface) {
    // init mutex 
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

    // init ui update thread
    void *updateUIRoutine(void *arg);
    ret = pthread_create(&ui_thread, NULL, updateUIRoutine, NULL);
    if (ret != 0) {
        perror("call pthread_create failed");
        endwin();
        exit(-1);
    }
}

void exitUI() {
    // end ncurses mode
    endwin();

    pthread_mutex_destroy(&mutex);
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

    int ret = pthread_mutex_lock(&mutex);
    if (ret != 0) {
        perror("call pthread_mutex_lock failed");
        exitUI();
        exit(-1);
    }

    f1.addData(dp, now);
    f5.addData(dp, now);
    f10.addData(dp, now);

    s1.addData(dp, now);
    s5.addData(dp, now);
    s10.addData(dp, now);

    ret = pthread_mutex_unlock(&mutex);
    if (ret != 0) {
        perror("call pthread_mutex_unlock failed");
        exitUI();
        exit(-1);
    }
}

void *updateUIRoutine(void *arg) {
    while (true) {
        sleep(1);

    }
}



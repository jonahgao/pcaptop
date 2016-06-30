#include "common.h"
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

std::ostream& operator<<(std::ostream&os, const DataPoint& dp) {
    os << "{ " << dp.ip << ":" << dp.port << ", " << (dp.direc == IN ? "IN" : "OUT")
        << ", " << dp.pktlen << ", " ;

    os << "0x" << std::hex << static_cast<int>(dp.flags) << " }";
    os << std::dec;

    return os;
}

time_t getCurrentSeconds() {
    struct timespec ts;
    int ret = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (ret != 0) {
        perror("call clock_gettime failed");
        exit(-1);
    }
    return ts.tv_sec;
}

std::string perfectFlowValue(uint64_t value) {
    static const uint64_t KB = 1 << 10;
    static const uint64_t MB = 1 << 20;
    static const uint64_t GB = 1 << 30;

    char buf[25] = {'\0'};

    if (value >= GB)
        snprintf(buf, 25, "%.3f%s", static_cast<double>(value) / GB, "GB");
    else if (value >= MB)
        snprintf(buf, 25, "%.3f%s", static_cast<double>(value) / MB, "MB");
    else if (value >= KB)
        snprintf(buf, 25, "%.3f%s", static_cast<double>(value) / KB, "KB");
    else
        snprintf(buf, 25, "%lu%s", value, "B");

    return buf;
}

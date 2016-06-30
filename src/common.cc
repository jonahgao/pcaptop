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

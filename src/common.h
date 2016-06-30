#ifndef COMMON_H_Q8O1RUBZ
#define COMMON_H_Q8O1RUBZ
#include <iostream>
#include <stdint.h>
#include <time.h>

enum Direction {
    IN,
    OUT,
};

struct DataPoint {
    std::string ip;       // client ip
    uint16_t port;        // client port
    Direction direc;      // direction
    int pktlen;           // packet length
    unsigned char flags;  // tcp flags
};


std::ostream& operator<<(std::ostream&os, const DataPoint& dp);

time_t getCurrentSeconds();


#endif /* end of include guard: COMMON_H_Q8O1RUBZ */

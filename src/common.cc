#include "common.h"
#include <iomanip>

std::ostream& operator<<(std::ostream&os, const DataPoint& dp) {
    os << "{ " << dp.ip << ":" << dp.port << ", " << (dp.direc == IN ? "IN" : "OUT")
        << ", " << dp.pktlen << ", " ;

    os << "0x" << std::hex << static_cast<int>(dp.flags) << " }";
    os << std::dec;

    return os;
}

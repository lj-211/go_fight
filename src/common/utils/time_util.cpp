#include "utils/time_util.h"

#include <stdio.h>
#include <time.h>

namespace util {
namespace go_time {

time_t get_u_time(void) {
    struct timeval t;
    int ret = gettimeofday(&t, NULL);
    if (ret == -1) {
        return -1;
    }

    return t.tv_sec * 1000000 + t.tv_usec;
}

time_t get_m_time(void) {
    struct timeval t;
    int ret = gettimeofday(&t, NULL);
    if (ret == -1) {
        return -1;
    }

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

time_t get_s_time(void) {
    time_t tm;
    tm = time(NULL);
    return tm;
}

} // end namespace time
} // end namespace util


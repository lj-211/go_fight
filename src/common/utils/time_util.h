#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <sys/time.h>

namespace util {
namespace go_time {

time_t get_u_time(void);

time_t get_m_time(void);

time_t get_s_time(void);

} // end namespace time
} // end namespace util

#endif

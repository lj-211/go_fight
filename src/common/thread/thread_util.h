#ifndef THREAD_UTIL_H
#define THREAD_UTIL_H

#include <pthread.h>

namespace thread {
bool create_worker(pthread_t& id, void *(*func)(void *), void *arg);
} // end namespace thread

#endif

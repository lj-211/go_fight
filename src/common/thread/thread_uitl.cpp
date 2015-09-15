#include "thread/thread_util.h"

#include "go_define.h"

#include <stdio.h>

namespace {
log_error thread_log_error = NULL;
log_trace thread_log_trace = NULL;
} // end namespace annoymous

namespace thread {
bool create_worker(pthread_t& id, void *(*func)(void *), void *arg) {
    bool ret = true;

    do {
        pthread_t thread;
        pthread_attr_t attr;
        int result;
        
        pthread_attr_init(&attr);

        if ((result = pthread_create(&thread, &attr, func, arg)) != 0) {
            ret = false;
            if (thread_log_error)
            {
                char tmp[256];
                snprintf(tmp, 256, "创建线程失败");
                thread_log_error(tmp);
            }
        }

        if (thread_log_trace)
        {
            char tmp[256];
            snprintf(tmp, 256, "创建线程成功,线程ID: %lld", thread);
            thread_log_trace(tmp);
        }
        id = thread;
    } while (false);

    return ret;
}
} // end namespace thread

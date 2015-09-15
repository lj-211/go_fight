#include "logger/logger.h"

#include "thread/thread_sync.h"
#include "utils/time_util.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

extern bool init_system();
extern bool init_config();
extern void logic_update(time_t now, time_t delta);
extern void deinit_all();

namespace {
// 在初始化完成后,逻辑人员负责将running状态改为true
volatile bool running = true;
thread::Mutex s_running_mutex;

volatile bool s_init_ok = false;
thread::Mutex s_init_mutex;

time_t s_last_frame = 0;
};

void set_init_ok(bool val) {
    thread::AutoLockMutex lock_run(&s_init_mutex);
    s_init_ok = val;
}

bool is_init_ok() {
    thread::AutoLockMutex lock_run(&s_init_mutex);
    return s_init_ok;
}

void stop_run() {
    thread::AutoLockMutex lock_run(&s_running_mutex);
    running = false;
    TRACE_LOG_NOARG("[process]停止运行进程");
}

bool is_running() {
    thread::AutoLockMutex lock_run(&s_running_mutex);
    return running;
}

void go_running() {
    thread::AutoLockMutex lock_run(&s_running_mutex);
    running = true;
}

int main() {
    // 1. do_init
    if (init_system() == false) {
        ERROR_LOG_NOARG("[process]初始化系统失败");
        printf("[process]初始化系统失败\n");
        return -1;
    }
    if (init_config() == false) {
        ERROR_LOG_NOARG("[process]初始化配置失败");
        printf("[process]初始化配置失败\n");
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    // 2. running loop
    while (is_running()) {
        time_t now = util::go_time::get_u_time();
        time_t delta = s_last_frame == 0 ? 0 : (now - s_last_frame);

        if (is_init_ok()) {
            logic_update(now, delta);
            s_last_frame = now;
        } else {
            usleep(1000 * 1000);
        }
    }
    // 3. de_init
    deinit_all();
}

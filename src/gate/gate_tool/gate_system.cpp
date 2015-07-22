#include "logger/logger.h"

#include <stdio.h>

bool init_system() {
    if (logger::init_logger("./config/log.conf") == false) {
        printf("[init]初始化日志系统失败");
        return false;
    }

    DEBUG_LOG("[init]%s", "初始化日志系统成功");

    return true;
}

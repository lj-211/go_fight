#include "logger/logger.h"
#include "database/redis/go_redis.h"
#include "game_redis.h"

#include <stdio.h>



bool init_system() {
    if (logger::init_logger("./config/log.conf") == false) {
        printf("[init]初始化日志系统失败");
        return false;
    }

    DEBUG_LOG("[init]%s", "初始化日志系统成功");

    if (redis_init() == false) {
        printf("[init]初始化Redis失败");
        return false;
    }
    DEBUG_LOG("[init]%s", "初始化Redis成功");

    return true;
}

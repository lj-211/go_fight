#include "data_redis.h"
#include "protocol.h"
#include "database/mysql/go_db.h"
#include <unistd.h>
#include <stdio.h>

extern MYSQL *g_mysql_handle;

namespace {
redisContext * s_redis_handle = NULL;
}

bool redis_init() {
    s_redis_handle = go::redis_connect("127.0.0.1", 6379, "", "");
    if (s_redis_handle) {
        DEBUG_LOG("[init]%s", "初始化Redis成功");
        return true;
    }
    printf("[init]初始化Redis失败");
    return false;
}

bool redis_cmd(std::string cmd, std::string &response) {
    return go::redis_cmd(s_redis_handle, cmd, response);

}

void* process_redis_list(redisContext* handle) {
    TRACE_LOG_NOARG("[redis]Redis队列线程启动");
    std::string cmd;
    std::string cmd_del;
    std::string response;
    std::string response_del;
    cmd = "ZRANGE role_sets 0 0";
    while(true) {
        if (redis_cmd(cmd, response)) {
            // 查询value,并存入MySql
            if (!response.empty()) {
                TRACE_LOG("[redis], Redis存盘队列中查询到需存入MySql的数据[%s]", response.c_str());
                Role role;
                if ((get_redis_data(response, &role))) {
                    char sql[2048];
                    snprintf(sql, sizeof(sql),"REPLACE INTO `role` (`id`, `name`, `user`) VALUES ('%s', '%s', '%s');",
                        role.id().c_str(), role.name().c_str(), role.user().c_str());
                    DEBUG_LOG("[db]save role sql[%s]", sql);
                    if (go::db_exe(g_mysql_handle, sql)) {
                        cmd_del = "ZREMRANGEBYRANK  role_sets 0 0";
                        redis_cmd(cmd_del, response_del);
                    }
                }
                response.clear();
            }
            else {
                TRACE_LOG_NOARG("[redis] Redis存盘队列中没有将存的数据");
            }
        } else {
            ERROR_LOG("[redis], redis命令执行错误, cmd[%s], error[%s]",
                      cmd.c_str(), response.c_str());
        }
        usleep(1000 * 1000);
    }

    return NULL;
}

void* process_redis_thread(void* prt) {
    process_redis_list(s_redis_handle);
    return NULL;
}

bool get_redis_data(std::string key, ProtoData *pd) {
    go::redis_get_data(s_redis_handle, key, pd);
    return true;
}
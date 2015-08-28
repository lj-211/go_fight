#include "game_redis.h"

namespace {
    LIST(RedisData*) s_redis_list;
    thread::Mutex s_redis_list_lock;
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

bool set_redis_data(std::string key, ProtoData *pd) {
    go::redis_set_data(s_redis_handle, key, pd);
    return true;
}

bool get_redis_data(std::string key, ProtoData *pd) {
    go::redis_get_data(s_redis_handle, key, pd);
    return true;
}

void* process_redis_list(redisContext* handle) {
    TRACE_LOG_NOARG("[redis]Redis队列线程启动");
    while (true) {
        static const size_t NUM_RECORD = 300;
        size_t i = 0;

        while (s_redis_list.size() > 0 && i < NUM_RECORD) {
            LIST(RedisData*)::iterator begin = s_redis_list.begin();
            TRACE_LOG("[redis]s_redis_list.size[%d]", s_redis_list.size());

            RedisData* redis_data = static_cast<RedisData*>(*(begin));
            s_redis_list.erase(begin);
            if (go::redis_set_data(handle, redis_data->key_, redis_data->value_) == false) {
                ERROR_LOG("[redis]redis写入失败, key[%s]", redis_data->key_.c_str());
            }
            i++;
            GO_DELETE(redis_data, RedisData);
        }
        usleep(100 * 1000);
    }
    return NULL;
}

void* process_redis_thread(void* prt) {
    process_redis_list(s_redis_handle);
    return NULL;
}

void add_redis_data(RedisData* redis_data) {
    s_redis_list_lock.Hold();
    s_redis_list.push_back(redis_data);
    s_redis_list_lock.Release();
}

void redis_cmd(std::string cmd) {
    std::string response;
    go::redis_cmd(s_redis_handle, cmd, response);

}
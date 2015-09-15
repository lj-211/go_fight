#include "database/redis/go_redis.h"
#include "go_define.h"
#include "logger/logger.h"
#include <unistd.h>
#include <iostream>

bool redis_init();
bool set_redis_data(std::string key, ProtoData *pd);
bool get_redis_data(std::string key, ProtoData *pd);

void add_redis_data(RedisData* redis_data);
void* process_redis_thread(void* prt);
bool redis_cmd(std::string cmd, std::string& response);

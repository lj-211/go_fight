#ifndef DATA_REDIS_H
#define DATA_REDIS_H

#include "database/redis/go_redis.h"
#include "logger/logger.h"
#include <iostream>

bool redis_init();
bool redis_cmd(std::string cmd, std::string &response);
void* process_redis_thread(void* prt);
bool get_redis_data(std::string key, ProtoData *pd);

#endif //DATA_REDIS_H

#ifndef GO_REDIS_H
#define GO_REDIS_H

#include "go_define.h"
#include "net/go_net.h"
#include "hiredis/hiredis.h"
#include <mysql/mysql.h>
#include "google/protobuf/message.h"

namespace go {
bool redis_init();

bool redis_deinit();

redisContext* redis_connect(const char* ip, unsigned int port, const char* user,
    const char* passwd);

bool redis_set_data(redisContext* handle, std::string key, ProtoData *pd);

bool redis_get_data(redisContext* handle, std::string key, ProtoData *pd);

void add_redis_data(RedisData* redis_data);

bool redis_cmd(redisContext* handle, std::string cmd, std::string &response);

} // end namespace go
#endif

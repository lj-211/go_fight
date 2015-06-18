#include "hiredis/hiredis.h"

#include "logger/logger.h"

bool test_use_redis_for_config() {
	const char* addr = "127.0.0.1";
	int port = 6379;

	redisContext* redis = NULL;
	redisReply* redis_ret = NULL;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	redis = redisConnectWithTimeout(addr, port, timeout);

	if (redis == NULL || redis->err) {
		if (redis == NULL) {
			ERROR_LOG_NOARG("分配redis连接错误");		
		} else {
			ERROR_LOG("连接出错,错误原因%s", redis->errstr);
		}

		return false;
	}

	redis_ret = (redisReply*)redisCommand(redis, "hget db_server_config:mysql ip");
	TRACE_LOG("数据库地址:%s", redis_ret->str);
    freeReplyObject(redis_ret);

	redis_ret = (redisReply*)redisCommand(redis, "hget db_server_config:mysql port");
	TRACE_LOG("数据库端口:%s", redis_ret->str);
    freeReplyObject(redis_ret);

	redis_ret = (redisReply*)redisCommand(redis, "hget db_server_config:mysql user");
	TRACE_LOG("数据库用户:%s", redis_ret->str);
    freeReplyObject(redis_ret);

	redis_ret = (redisReply*)redisCommand(redis, "hget db_server_config:mysql passwd");
	TRACE_LOG("数据库密码:%s", redis_ret->str);
    freeReplyObject(redis_ret);

	redisFree(redis);

	return true;
}

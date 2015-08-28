#include "database/redis/go_redis.h"
#include "go_define.h"
#include "logger/logger.h"
#include "utils/str_util.h"
#include <unistd.h>



namespace go {

redisContext* redis_connect(const char* ip, unsigned int port, const char* user,
    const char* passwd) {
    redisContext * handle = NULL;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    handle = redisConnectWithTimeout(ip, port, timeout);

    if (handle == NULL || handle->err) {
        if (handle == NULL) {
            ERROR_LOG_NOARG("[redis]分配redis连接错误");        
        } else {
            ERROR_LOG("[redis]连接出错,错误原因[%s]", handle->errstr);
        }
        return NULL;
    }

    return handle;
}

bool redis_set_data(redisContext* handle, std::string key, ProtoData *pd) {

    redisReply* redis_ret = NULL;
    
    // 将对象以二进制保存
    const int byte_size = pd->ByteSize();
    char* buf = new char[byte_size];
    bzero(buf, byte_size);
    pd->SerializeToArray(buf, byte_size);

    // 将对象写入redis数据库
    redis_ret = (redisReply*) redisCommand(handle, "SET %b %b", 
        key.c_str(), (size_t) key.length(), buf, (size_t)byte_size);

    if (redis_ret == NULL) {
        ERROR_LOG_NOARG("[redis]命令执行失败");
        return false;
    }

    if (redis_ret->type == REDIS_REPLY_ERROR) {
        ERROR_LOG("[redis]写入失败, type[%d], errstr[%s]", redis_ret->type, redis_ret->str);
        return false;
    }

    freeReplyObject(redis_ret);

    TRACE_LOG("[redis]写入成功!, key[%s]", key.c_str());
    return true;
}

bool redis_get_data(redisContext* handle, std::string key, ProtoData *pd) {

    redisReply* redis_ret = NULL;

    // 从redis数据库读取对象数据
    redis_ret = (redisReply*)redisCommand(handle, "GET %s", key.c_str()); 

    if (redis_ret == NULL) {
        ERROR_LOG_NOARG("[redis]命令执行失败");
        return false;
    }

    if (redis_ret->type == REDIS_REPLY_ERROR) {
        ERROR_LOG("[redis]读出失败,错误原因[%s]", redis_ret->str);
        return false;
    }

    if (redis_ret->type == REDIS_REPLY_NIL) {
        ERROR_LOG("[redis]访问的数据不存在, key[%s]", key.c_str());
        return false;
    }

    pd->ParseFromArray(redis_ret->str, redis_ret->len);
    freeReplyObject(redis_ret);

    TRACE_LOG("[redis]读出成功, key[%s]", key.c_str());
    return true;
}

bool redis_cmd(redisContext* handle, std::string cmd, std::string &response) {
    redisReply* reply = NULL;
    reply = (redisReply*)redisCommand(handle, cmd.c_str());
    bool ret = false;
    redisReply **element_reply = NULL;

    if (reply == NULL) {
        ERROR_LOG("[redis]命令执行失败, cmd[%s]", cmd.c_str());
        return false;
    }

    switch (reply->type) {
        case REDIS_REPLY_INTEGER:
            response = util::str::int_to_str(reply->integer);
            ret = true;
            break;
        case REDIS_REPLY_STRING:
            response = reply->str;
            ret = true;
            break;
        case REDIS_REPLY_STATUS:
            response = reply->str;
            ret = true;
            break;
        case REDIS_REPLY_NIL:
            response = "";
            ret = true;
            break;
        case REDIS_REPLY_ARRAY:
            element_reply = reply->element;
            if (element_reply && *element_reply) {
                response = (*element_reply)->str;
            }
            ret = true;
            break;
        case REDIS_REPLY_ERROR:
            ERROR_LOG("[redis]写入失败, cmd[%s], type[%d], error[%s]",
                      cmd.c_str(), reply->type, reply->str);
            response = reply->str;
            ret = false;
            break;
        default:
            response = "Undefine Reply Type";
            ERROR_LOG("[redis]写入失败, 不支持的Redis类型, cmd[%s]", cmd.c_str());
            ret = false;
            break;
    }

    freeReplyObject(reply);

    TRACE_LOG("[redis]写入成功!, cmd[%s]", cmd.c_str());
    return ret;
}


} // end namespace go

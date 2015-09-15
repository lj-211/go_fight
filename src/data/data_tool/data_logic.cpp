#include "logger/logger.h"
#include "script/lua_script.h"
#include "utils/str_util.h"
#include "net/go_net.h"
#include "thread/thread_util.h"
#include "go_define.h"
#include "database/redis/go_redis.h"
#include "database/mysql/go_db.h"
#include "data_redis.h"
#include "protocol.h"

#include "LuaBridge/LuaBridge.h"
#include "hiredis/hiredis.h"

#include <sstream>
#include <unistd.h>

struct DataConfig {
    int server_id_;
    std::string redis_addr_;
    int port_;
};

struct DataInternalConfig {
    std::string data_ip_;
    int data_port_;
    std::string db_ip_;
    int db_port_;
    std::string db_user_;
    std::string db_passwd_;
    std::string db_;
};

MYSQL* g_mysql_handle = NULL;


namespace {
const char* s_config_file = "./script/config/data_init.lua";

void regist_game_config(lua_State* state) {
    luabridge::getGlobalNamespace(state)
        .beginClass<DataConfig>("DataConfig")
        .addData("server_id_", &DataConfig::server_id_)
        .addData("redis_addr_", &DataConfig::redis_addr_)
        .addData("port_", &DataConfig::port_)
        .endClass();
}

static DataConfig s_data_config;
static DataInternalConfig s_data_internal_config;

MAP(int, uint64_t) s_connections_to_me;

DEQUE(net::MsgNode*) s_recv_msgs;
} // annoymous namespace

bool register_server(int id, uint64_t conn) {
    MAP(int, uint64_t)::iterator iter = s_connections_to_me.find(id);
    if (iter != s_connections_to_me.end()) {
        ERROR_LOG("ID为%d的服务器连接已经存在于列表中,不可注册", id);
        return false;
    }

    s_connections_to_me[id] = conn;
    return true;
}

void disconnect_server(int id) {
    MAP(int, uint64_t)::iterator iter = s_connections_to_me.find(id);
    if (iter != s_connections_to_me.end()) {
        s_connections_to_me.erase(iter);
    } else {
        ERROR_LOG("ID为%d的服务器连接不在管理中,请检查是否已释放", id);
    }
}

bool init_lua_config() {
    bool ret = true;

    lua_State* vm = luaL_newstate();
    luaL_openlibs(vm);

    regist_game_config(vm);

    luabridge::setGlobal(vm, &s_data_config, "s_data_config");
    if (luaL_dofile(vm, s_config_file) != 0) {
        ERROR_LOG("[init]初始化DS配置错误,原因是: %s", lua_tostring(vm, -1));
        ret = false;
    } else {
        TRACE_LOG("[init]初始化DS配置成功,网关id: %d, redis地址: %s, redis端口: %d",
            s_data_config.server_id_, s_data_config.redis_addr_.c_str(),
            s_data_config.port_);
    }

    lua_close(vm);

    return ret;
}

bool init_net_config() {
    const char* addr = s_data_config.redis_addr_.c_str();
    int port = s_data_config.port_;

    redisContext* redis = NULL;
    redisReply* redis_ret = NULL;

    struct timeval timeout = {1, 500000};
    redis = redisConnectWithTimeout(addr, port, timeout);

    if (redis == NULL || redis->err) {
        if (redis == NULL) {
            ERROR_LOG_NOARG("分配redis连接错误");        
        } else {
            ERROR_LOG("连接出错,错误原因%s", redis->errstr);
        }

        return false;
    }

    std::stringstream name;
    name << "hget " << "data_" << s_data_config.server_id_ << ":net ";
    std::string prefix = name.str();

    s_data_internal_config.data_ip_ = util::str::BlankStr();
    s_data_internal_config.data_port_ = -1;

    bool init_ok = true;
    do {
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "ip").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.data_ip_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }

        redis_ret = (redisReply*)redisCommand(redis, (prefix + "port").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.data_port_ = util::str::str_to_int(redis_ret->str);
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "db_ip").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.db_ip_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "db_port").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.db_port_ = util::str::str_to_int(redis_ret->str);
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "db_user").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.db_user_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "db_passwd").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.db_passwd_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "db").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_data_internal_config.db_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }
    } while (false);

    redisFree(redis);

    return init_ok;
}

bool init_net() {
    net::regist_net_log(&logger::error_log, &logger::trace_log);

    net::NetSetting ns;
    ns.max_connections_ = 1024 * 4;
    ns.ip_addr_ = s_data_internal_config.data_ip_;
    ns.listen_port_ = s_data_internal_config.data_port_;
    // 数据服务器连接数很少,只使用一个网络线程
    ns.net_thread_num_ = 1;

    if (!net::net_pre_set_parameter(ns)) {
        return false;
    }

    if (!net::net_init()) {
        return false;
    }

    return true;
}

bool init_db() {
    
    const char* ip = s_data_internal_config.db_ip_.c_str();
    unsigned int port = s_data_internal_config.db_port_;
    const char* user = s_data_internal_config.db_user_.c_str();
    const char * passwd = s_data_internal_config.db_passwd_.c_str();
    const char* db = s_data_internal_config.db_.c_str();

    g_mysql_handle = go::db_connect(ip, port, user, passwd, db);

    if (g_mysql_handle) {
        return true;
    }

    return false;
}

extern void set_init_ok(bool val);
bool init_config() {
    do {
        if (init_lua_config() == false) {
            ERROR_LOG("%s", "初始化DS的lua配置错误");
            break;
        }

        if (init_net_config() == false) {
            ERROR_LOG("%s", "初始化DS的网络配置错误");
            break;
        }

        if (init_net() == false) {
            ERROR_LOG("%s", "初始化网络失败");
            break;
        }

        if (init_db() == false) {
            ERROR_LOG("%s", "初始化网络失败");
            break;
        }

        // Redis列表线程
        pthread_t id;
        thread::create_worker(id, &process_redis_thread, NULL);

        protocol_init();

        set_init_ok(true);

        return true;
    } while (false);

    return false;
}

void logic_update(time_t now, time_t delta) {
    usleep(20 * 1000);
    
    // 1. process msg
    net::get_net_msgs(s_recv_msgs);    
    if (s_recv_msgs.size() > 0) {
        DEBUG_LOG("[net]收到网络消息%d", s_recv_msgs.size());
    }
    while (s_recv_msgs.size() > 0) {
        net::MsgNode* msg_node = s_recv_msgs.front();
        s_recv_msgs.pop_front();
    
        // delete msg node in message_process
        net::message_process(msg_node);
    }
}

void deinit_all() {
    TRACE_LOG("%s", "退出程序");
}



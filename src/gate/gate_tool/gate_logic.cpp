#include "logger/logger.h"
#include "script/lua_script.h"
#include "utils/str_util.h"
#include "net/go_net.h"
#include "thread/thread_util.h"

#include "protocol.h"

#include "hiredis/hiredis.h"
#include <unistd.h>

struct GateConfig {
    int server_id_;
    std::string redis_addr_;
    int port_;
};

struct GameSerInfo {
    std::string gs_ip_;
    int gs_port_;
};

namespace {
const char* s_config_file = "./script/config/gate_init.lua";
std::string s_server_net_ip = "";
int s_server_port = 0;
int s_net_thread_num = 1;

GameSerInfo s_gameser_info;

void regist_gate_config(lua_State* state) {
    luabridge::getGlobalNamespace(state)
        .beginClass<GateConfig>("GateConfig")
        .addData("server_id_", &GateConfig::server_id_)
        .addData("redis_addr_", &GateConfig::redis_addr_)
        .addData("port_", &GateConfig::port_)
        .endClass();
}

GateConfig s_gate_config;

MAP(int, uint64_t) s_connections_to_me;

DEQUE(net::MsgNode*) s_recv_msgs;
} // end namespace annoymous

uint64_t s_game_conn = 0;

extern void set_init_ok(bool val);
void* connect_to_gs(void* arg);
void regist_to_game();
void connect_gs();
void* connector_callback(uint64_t conn, bool is_ok) {
    ERROR_LOG("收到连接器回调: %lld, 逻辑服务器为: %lld", conn, s_game_conn);
    if (conn == s_game_conn) {
        if (is_ok) {
            TRACE_LOG("连接器连接成功: %lld, 发起注册", conn);
            set_init_ok(true);
            regist_to_game();            
        } else {
            connect_gs();
        }
    }

    return NULL;
}

bool register_client(int id, uint64_t conn) {
    MAP(int, uint64_t)::iterator iter = s_connections_to_me.find(id);
    if (iter != s_connections_to_me.end()) {
        ERROR_LOG("ID为%d的客户端连接已经存在于列表中,不可注册", id);
        return false;
    }

    s_connections_to_me[id] = conn;
    return true;
}

void disconnect_client(int id) {
    MAP(int, uint64_t)::iterator iter = s_connections_to_me.find(id);
    if (iter != s_connections_to_me.end()) {
        s_connections_to_me.erase(iter);
    } else {
        ERROR_LOG("ID为%d的客户端连接不在管理中,请检查是否已释放", id);
    }
}

bool init_lua_config() {
    bool ret = true;

    lua_State* vm = luaL_newstate();
    luaL_openlibs(vm);

    regist_gate_config(vm);

    luabridge::setGlobal(vm, &s_gate_config, "s_gate_config");
    if (luaL_dofile(vm, s_config_file) != 0) {
        ERROR_LOG("[init]初始化网关配置错误,原因是: %s", lua_tostring(vm, -1));
        ret = false;
    } else {
        TRACE_LOG("[init]初始化网关配置成功,网关id: %d, redis地址: %s, redis端口: %d",
            s_gate_config.server_id_, s_gate_config.redis_addr_.c_str(),
            s_gate_config.port_);
    }

    lua_close(vm);

    return ret;
}

bool init_net_config() {
    const char* addr = s_gate_config.redis_addr_.c_str();
    int port = s_gate_config.port_;

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
    name << "hget " << "gate_" << s_gate_config.server_id_ << ":net ";
    std::string prefix = name.str();

    s_server_net_ip = util::str::BlankStr();
    s_server_port = 0;
    bool init_ok = true;

    do {
        redis_ret = (redisReply*)redisCommand(redis, (prefix + "ip").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_server_net_ip = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }

        redis_ret = (redisReply*)redisCommand(redis, (prefix + "port").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_server_port = util::str::str_to_int(redis_ret->str);
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }

        redis_ret = (redisReply*)redisCommand(redis, (prefix + "net_thread_num").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL &&
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_net_thread_num = util::str::str_to_int(redis_ret->str);
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }

        redis_ret = (redisReply*)redisCommand(redis, (prefix + "gs_ip").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_gameser_info.gs_ip_ = redis_ret->str;
            freeReplyObject(redis_ret);
        } else {
            init_ok = false;
        }

        redis_ret = (redisReply*)redisCommand(redis, (prefix + "gs_port").c_str());
        if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
            redis_ret->type != REDIS_REPLY_ERROR) {

            s_gameser_info.gs_port_ = util::str::str_to_int(redis_ret->str);
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
    ns.ip_addr_ = s_server_net_ip;
    ns.listen_port_ = s_server_port;
    ns.net_thread_num_ = s_net_thread_num;

    if (!net::net_pre_set_parameter(ns)) {
        return false;
    }

    if (!net::net_init()) {
        return false;
    }

    return true;
}

void* connect_to_gs(void* arg) {
while (!s_game_conn) {
        s_game_conn = net::net_connect("GS", s_gameser_info.gs_ip_.c_str(),
                                       s_gameser_info.gs_port_);
        usleep(2000*1000);
    }
    return NULL;
}

bool init_config() {
    do {
        if (init_lua_config() == false) {
            ERROR_LOG("%s", "初始化lua配置错误");
            break;
        }

        if (init_net_config() == false) {
            ERROR_LOG("%s", "初始化网络配置失败");
            break;
        }

        if (init_net() == false) {
            ERROR_LOG("%s", "初始化网络失败");
            break;
        }

        protocol_init();
        net::regist_connector_callback(connector_callback);

        connect_gs();

        return true;
    } while (false);

    return false;
}

void logic_update(time_t now, time_t delta) {
    usleep(20 * 1000);

    // 1. process msg
    net::get_net_msgs(s_recv_msgs);    
    if (s_recv_msgs.size() > 0) {
    //    DEBUG_LOG("收到网络消息%d", s_recv_msgs.size());
    }
    while (s_recv_msgs.size() > 0) {
        net::MsgNode* msg_node = s_recv_msgs.front();
        s_recv_msgs.pop_front();

        // 如果消息来自GS，则发给Client;如果消息来自Client，则发给GS
        if (msg_node->msg_conn_ == (net::Connection*)s_game_conn) {
            if (msg_node->msg_type_ != 0) {
        //        TRACE_LOG("转发给Client, cid[%lld]", msg_node->cli_conn_);
                net::net_send(msg_node->cli_conn_, msg_node);
            } else {
                s_game_conn = 0;
                net::message_process(msg_node);
            }
        } else {
            if (msg_node->msg_type_ == 0) {
                TRACE_LOG("客户端断开，连接[%lld]", msg_node->msg_conn_);
            }
         //   TRACE_LOG("转发给GS, cid[%lld], gsid[%lld]", msg_node->msg_conn_, s_game_conn);
            msg_node->cli_conn_ = (uint64_t) msg_node->msg_conn_;
            net::net_send(s_game_conn, msg_node);

        }
        // delete msg node in message_process
        // net::message_process(msg_node);
    }
}

void deinit_all() {
    TRACE_LOG("%s", "退出程序");
}

void connect_gs() {
    pthread_t id;
    thread::create_worker(id, &connect_to_gs, &s_gameser_info);
}
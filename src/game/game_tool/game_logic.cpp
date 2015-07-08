#include "logger/logger.h"
#include "script/lua_script.h"
#include "utils/str_util.h"
#include "net/go_net.h"
#include "thread/thread_util.h"

#include "LuaBridge/LuaBridge.h"
#include "hiredis/hiredis.h"

#include <sstream>
#include <unistd.h>

struct GameConfig {
	int server_id_;
	std::string redis_addr_;
	int port_;
};

struct GameInternalConfig {
	std::string game_ip_;
	int game_port_;
};

namespace {
const char* s_config_file = "./script/config/game_init.lua";

void regist_game_config(lua_State* state) {
	luabridge::getGlobalNamespace(state)
		.beginClass<GameConfig>("GameConfig")
		.addData("server_id_", &GameConfig::server_id_)
		.addData("redis_addr_", &GameConfig::redis_addr_)
		.addData("port_", &GameConfig::port_)
		.endClass();
}

GameConfig s_game_config;
GameInternalConfig s_game_internal_config;
} // annoymous namespace


bool init_lua_config() {
	bool ret = true;

	lua_State* vm = luaL_newstate();
	luaL_openlibs(vm);

	regist_game_config(vm);

	luabridge::setGlobal(vm, &s_game_config, "s_game_config");
	if (luaL_dofile(vm, s_config_file) != 0) {
		ERROR_LOG("[init]初始化网关配置错误,原因是: %s", lua_tostring(vm, -1));
		ret = false;
	} else {
		TRACE_LOG("[init]初始化网关配置成功,网关id: %d, redis地址: %s, redis端口: %d",
			s_game_config.server_id_, s_game_config.redis_addr_.c_str(),
			s_game_config.port_);
	}

	lua_close(vm);

	return ret;
}

bool init_net_config() {
	const char* addr = s_game_config.redis_addr_.c_str();
	int port = s_game_config.port_;

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
	name << "hget " << "game_" << s_game_config.server_id_ << ":net ";
	std::string prefix = name.str();

	s_game_internal_config.game_ip_ = util::str::BlankStr();
	s_game_internal_config.game_port_ = 0;

	bool init_ok = true;
	do {
		redis_ret = (redisReply*)redisCommand(redis, (prefix + "ip").c_str());
		if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
			redis_ret->type != REDIS_REPLY_ERROR) {

			s_game_internal_config.game_ip_ = redis_ret->str;
			freeReplyObject(redis_ret);
		} else {
			init_ok = false;
		}

		redis_ret = (redisReply*)redisCommand(redis, (prefix + "port").c_str());
		if (redis_ret && redis_ret->type != REDIS_REPLY_NIL && 
			redis_ret->type != REDIS_REPLY_ERROR) {

			s_game_internal_config.game_port_ = util::str::str_to_int(redis_ret->str);
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
	ns.ip_addr_ = s_game_internal_config.game_ip_;
	ns.listen_port_ = s_game_internal_config.game_port_;
	// 游戏服务器连接数很少,只使用一个网络线程
	ns.net_thread_num_ = 1;

	if (!net::net_pre_set_parameter(ns)) {
		return false;
	}

	if (!net::net_init()) {
		return false;
	}

	return true;
}

extern void set_init_ok(bool val);
bool init_config() {
	do {
		if (init_lua_config() == false) {
			ERROR_LOG("%s", "初始化GS的lua配置错误");
			break;
		}

		if (init_net_config() == false) {
			ERROR_LOG("%s", "初始化GS的网络配置错误");
		}

		if (init_net() == false) {
			ERROR_LOG("%s", "初始化网络失败");
			break;
		}

		// 这里暂时将服务器状态置为初始化完成
		// 实际需要和DB进行完全数据通信完成后
		set_init_ok(true);

		return true;
	} while (false);

	return false;
}

void logic_update(time_t now, time_t delta) {
	usleep(2000 * 1000);
	ERROR_LOG("%s", "请注意,这是GS的测试主循环日志,请删除我...");
}

void deinit_all() {
	TRACE_LOG("%s", "退出程序");
}

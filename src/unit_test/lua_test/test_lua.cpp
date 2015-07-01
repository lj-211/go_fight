extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"	
}

#include "LuaBridge/LuaBridge.h"

#include "script/lua_script.h"
#include "utils/str_util.h"

struct TestLua {
	int server_id_;
	std::string server_name_;
};

namespace {
TestLua s_test_lua;
int s_test_int;
}; // end annoymous namespace

void register_class(lua_State* state) {
	luabridge::getGlobalNamespace(state)
		.beginClass<TestLua>("TestLua")
		.addData("server_id_", &TestLua::server_id_)
		.addData("server_name_", &TestLua::server_name_)
		.endClass();
}

bool test_lua_load_config() {
	do {
		lua_State* vm = luaL_newstate();
		luaL_openlibs(vm);

		register_class(vm);

		s_test_lua.server_id_ = 1234;
		s_test_lua.server_name_ = "测试字符串";
		luabridge::setGlobal(vm, &s_test_lua, "s_test_lua");

		if (luaL_dofile(vm, "./script/config/init_config.lua") != 0) {
			printf("执行lua文件错误\n");
			lua_close(vm);
			break;
		} else {
			printf("执行lua文件成功\n");
		}

		printf("从lua返回后id: %d, server_name: %s\n%d\n", s_test_lua.server_id_, 
			s_test_lua.server_name_.c_str(), s_test_int);

		go::script::LuaArguments a;
		a.AddArgument(12);
		a.AddArgument(24);
		int ret;
		go::script::call_lua_func(vm, "test_error_function", a, ret);
		printf("函数返回结果是%d\n", ret);

		a.ClearArgument();
		a.AddArgument("abcd");
		std::string str_ret;
		go::script::call_lua_func(vm, "test_return_str", a, str_ret);
		printf("函数返回字符串结果%s\n", str_ret.c_str());

		str_ret = util::str::BlankStr();
		go::script::LuaTable tbl;
		go::script::LuaArg tmp;
		tmp.type_ = google::protobuf::FieldDescriptor::CPPTYPE_STRING;
		tmp.str_val_ = "1234";
		for (size_t i = 0; i < 10; ++i) {
			tbl.insert(std::pair<std::string, go::script::LuaArg>(util::str::int_to_str(i), tmp));
		}
		go::script::call_lua_func(vm, "test_parameter", tbl, str_ret);

		lua_close(vm);

		return true;

	} while (false);

	return false;
}

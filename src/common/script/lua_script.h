#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include "go_define.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"	
}

#include "LuaBridge/LuaBridge.h"

#include "logger/logger.h"

// protobuf
#include "google/protobuf/message.h"

#include <stdio.h>

namespace go {
namespace script {

int lua_traceback(lua_State* state) {
	lua_getglobal(state, "debug");
	lua_getfield(state, -1, "traceback");

    lua_pushvalue(state, 1);
    lua_pushinteger(state, 2);

    lua_pcall(state, 2, 1, 0);
	TRACE_LOG("[script]%s", lua_tostring(state, -1));	

	return 0;
}

void lua_stack_dump(lua_State* state) {
    int top = lua_gettop(state);
	TRACE_LOG_NOARG("[script]打印lua堆栈");
    for (int i = 1; i <= top; i++) {
		int t = lua_type(state, i);
		switch (t) {
			case LUA_TSTRING:
				TRACE_LOG("[script]栈%d: %s", i, lua_tostring(state, i));
				break;

			case LUA_TBOOLEAN:
				TRACE_LOG("[script]栈%d: %s", i, lua_toboolean(state, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:
				TRACE_LOG("[script]栈%d: %g", i, lua_tonumber(state, i));
				break;

			case LUA_TFUNCTION:
				TRACE_LOG("[script]栈%d: %d", i, lua_tocfunction(state, i));
				break;

			default:
				TRACE_LOG("[script]栈%d: 不可打印类型%d", i, t);
				break;
		}
	}
}

struct LuaArg {
	int type_; 
	union Data{
		int int_val_;
		float float_val_;
		bool bool_val_;
	} data_;
	std::string str_val_;
};

class LuaArguments {
public:
	LuaArguments() {
	}

	~LuaArguments() {
		ClearArgument();
	}

	void AddArgument(const char* val) {
		LuaArg* arg = GO_NEW(LuaArg);
		arg->type_ = google::protobuf::FieldDescriptor::CPPTYPE_STRING;
		arg->str_val_ = val;
		m_args_.push_back(arg);
	}

	void AddArgument(int val) {
		LuaArg* arg = GO_NEW(LuaArg);
		arg->type_ = google::protobuf::FieldDescriptor::CPPTYPE_INT32;
		arg->data_.int_val_ = val;
		m_args_.push_back(arg);
	}

	void AddArgument(float val) {
		LuaArg* arg = GO_NEW(LuaArg);
		arg->type_ = google::protobuf::FieldDescriptor::CPPTYPE_FLOAT;
		arg->data_.float_val_ = val;
		m_args_.push_back(arg);
	}

	void AddArgument(bool val) {
		LuaArg* arg = GO_NEW(LuaArg);
		arg->type_ = google::protobuf::FieldDescriptor::CPPTYPE_BOOL;
		arg->data_.bool_val_ = val;
		m_args_.push_back(arg);
	}

	void ClearArgument() {
		for (size_t i = 0; i < m_args_.size(); ++i) {
			GO_DELETE(m_args_[i], LuaArg);
		}

		m_args_.clear();
	}

public:
	VECTOR(LuaArg*) m_args_;
};

bool call_lua_func(lua_State* state, const char* func, const LuaArguments& args, int& ret) {
	do {
		lua_pushcfunction(state, lua_traceback);

		lua_getglobal(state, func);
		if (lua_type(state, -1) != LUA_TFUNCTION) {
			lua_pop(state, 1);
			ERROR_LOG("[script]找不到对应的函数%s", func);
			break;
		}

		size_t count = args.m_args_.size();
		for (size_t i = 0; i < count; ++i) {
			LuaArg* arg = args.m_args_[i];
			switch (arg->type_) {
				case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
					lua_pushstring(state, arg->str_val_.c_str());
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
					lua_pushnumber(state, arg->data_.int_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(state, arg->data_.float_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(state, arg->data_.bool_val_);
					break;
			}
		}

		if (lua_pcall(state, count, 1, -(2+count)) != 0) {
			ERROR_LOG("[script]调用lua函数出错,函数名为:%s", func);
			// 弹出函数返回值
			// 弹出错误处理函数的
			lua_pop(state, 2);
			break;
		}

		if (!lua_isnumber(state, -1)) {
			lua_pop(state, 2);
			break;
		}

		ret = lua_tointeger(state, -1);

		lua_pop(state, 2);

		return true;
	} while (false);

	return false;
}

bool call_lua_func(lua_State* state, const char* func, const LuaArguments& args, 
	std::string& ret) {
	do {
		lua_pushcfunction(state, lua_traceback);

		lua_getglobal(state, func);
		if (lua_type(state, -1) != LUA_TFUNCTION) {
			lua_pop(state, 1);
			ERROR_LOG("[script]找不到对应的函数%s", func);
			break;
		}

		size_t count = args.m_args_.size();
		for (size_t i = 0; i < count; ++i) {
			LuaArg* arg = args.m_args_[i];
			switch (arg->type_) {
				case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
					lua_pushstring(state, arg->str_val_.c_str());
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
					lua_pushinteger(state, arg->data_.int_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(state, arg->data_.float_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(state, arg->data_.bool_val_);
					break;
			}
		}

		if (lua_pcall(state, count, 1, -(2+count)) != 0) {
			ERROR_LOG("[script]调用lua函数出错,函数名为:%s", func);
			// 弹出错误处理函数的返回值
			lua_pop(state, 2);
			break;
		}

		if (!lua_isstring(state, -1)) {
			lua_pop(state, 2);
			break;
		}

		ret = lua_tostring(state, -1);

		lua_pop(state, 2);

		return true;

	} while (false);

	return false;
}

typedef MAP(std::string, LuaArg) LuaTable;
bool call_lua_func(lua_State* state, const char* func, const LuaTable& tbl, 
	std::string& ret) {
	do {
		lua_pushcfunction(state, lua_traceback);

		lua_getglobal(state, func);
		if (lua_type(state, -1) != LUA_TFUNCTION) {
			lua_pop(state, 1);
			ERROR_LOG("[script]找不到对应的函数%s", func);
			break;
		}

		lua_newtable(state);
		LuaTable::const_iterator tbl_iter = tbl.begin();
		while (tbl_iter != tbl.end()) {
			lua_pushstring(state, tbl_iter->first.c_str());
			const LuaArg* arg = &(tbl_iter->second);
			switch (arg->type_) {
				case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
					lua_pushstring(state, arg->str_val_.c_str());
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
					lua_pushnumber(state, arg->data_.int_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(state, arg->data_.float_val_);
					break;
				case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(state, arg->data_.bool_val_);
					break;
			}
			lua_settable(state, -3);

			++tbl_iter;
		}

		if (lua_pcall(state, 1, 1, -(2+1)) != 0) {
			ERROR_LOG("[script]调用lua函数出错,函数名为:%s", func);
			// 弹出错误处理函数的返回值
			lua_pop(state, 2);
			break;
		}

		if (!lua_isstring(state, -1)) {
			lua_pop(state, 2);
			break;
		}

		ret = lua_tostring(state, -1);

		lua_pop(state, 2);

		return true;

	} while (false);

	return false;
}

} // end namespace script
} // end namespace go

#endif

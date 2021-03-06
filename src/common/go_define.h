#ifndef GO_DEFINE_H
#define GO_DEFINE_H

#include "memory/mem_define.h"
#include "google/protobuf/message.h"

#define DISALLOW_COPY_AND_ASSIGN(T) \
    T(const T&); \
    void operator=(const T&)

typedef void (*log_error)(const char*);
typedef void (*log_trace)(const char*);

typedef void (*log_error_raw)(const char*, int, const char*, ...);
typedef void (*log_trace_raw)(const char*, int, const char*, ...);

typedef ::google::protobuf::Message ProtoDataContainer;
typedef ::google::protobuf::Message ProtoData;

struct RedisData {
    std::string key_;
    int type_;
    ProtoData* value_;

    RedisData()
        : key_(""),
        type_(0),
        value_(NULL) {
        }
};

// xxxx000
// 后三位为单服务器递增id
#define DB_SERVER 8
#define GAME_SERVER 16
#define GATE_SERVER 32

#endif

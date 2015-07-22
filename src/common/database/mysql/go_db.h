#ifndef GO_DB_H
#define GO_DB_H

// protobuf
#include "google/protobuf/message.h"

#include <mysql/mysql.h>

typedef ::google::protobuf::Message ProtoDataContainer;
typedef ::google::protobuf::Message ProtoData;

namespace go {

bool db_init();

bool db_deinit();

MYSQL* db_connect(const char* ip, unsigned int port, const char* user,
    const char* passwd, const char* db);

void db_close(MYSQL* handle);

// 用于没有返回结果的SQL
bool db_exe(MYSQL* handle, const char* cmd);

// 因为启用了多语句查询
// 本接口只处理第一条的结果
bool db_get_data(MYSQL* handle, const char* cmd, ProtoDataContainer* data);

} // end namespace go

#endif

#ifndef GO_DB_H
#define GO_DB_H

#include <mysql/mysql.h>

namespace go {

bool db_init();

bool db_deinit();

MYSQL* db_connect(const char* ip, unsigned int port, const char* user,
	const char* passwd, const char* db);

void db_close(MYSQL* handle);

bool db_exe(MYSQL* handle, const char* cmd);

} // end namespace go

#endif

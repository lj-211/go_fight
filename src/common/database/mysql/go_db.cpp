#include "database/mysql/go_db.h"

#include "logger/logger.h"

namespace go {

bool db_init() {
	// 
	return true;
}

bool db_deinit() {
}

MYSQL* db_connect(const char* ip, unsigned int port, const char* user,
	const char* passwd, const char* db) {
	MYSQL* ret = mysql_init(NULL);

	do {
		char value = 1;
		mysql_options(s_mysql, MYSQL_OPT_RECONNECT, (char*)&value);
		mysql_options(s_mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		MYSQL* return_val = mysql_real_connect(ret, ip, user, passwd, 
			db, port, NULL, CLIENT_MULTI_STATEMENTS);	
		
		if (return_val == NULL) {
			ERROR_LOG("连接mysql失败,连接信息 ip:%s port:%d user:%s passwd:%s db:%s",
				ip, port, user, passwd, db);
			break;
		}

		return ret;
	} while (false);

	mysql_close(ret);
	return NULL;
}

void db_close(MYSQL* handle) {
	if (handle == NULL) {
		return;
	}

	mysql_close(handle);
}

bool db_exe(MYSQL* handle, const char* cmd) {
	if (handle == NULL || cmd == NULL) {
		return false;
	}

	do {
		int ret = mysql_query(cmd);
		if (ret != 0) {
			ERROR_LOG("[db]执行数据库命令失败,命令%s,失败原因%s", cmd, mysql_error(handle));
			break;
		}

		MYSQL_RES* res = mysql_store_result(handle);
		if (res != NULL) {
			mysql_free_result(res);
		}

		while (!mysql_next_result(handle)) {
			res =  mysql_store_result(handle);
			if (!res) {
				mysql_free_result(res);
			}
		}

		return true;
	} while (false);

	return false;
}

} // end namespace go

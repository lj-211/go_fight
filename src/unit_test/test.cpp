#include <stdio.h>

bool test_log4cplus();
bool test_memory();
bool test_net();
bool test_database();
bool test_use_redis_for_config();

int main() {
	printf("process: unit_test\n");

	if (!test_log4cplus()) {
		printf("%s", "测试日志系统失败");
	}

	if (false && !test_memory()) {
		printf("%s", "测试内存池工具失败");
	}

	if (false && !test_net()) {
		printf("%s", "测试网络模块错误");
	}

	if (false && !test_database()) {
		printf("%s", "测试数据库模块错误");
	}

	if (!test_use_redis_for_config()) {
		printf("%s", "测试redis数据库作为配置错误");
	}

	return 0;
}

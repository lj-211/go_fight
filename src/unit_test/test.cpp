#include <stdio.h>

bool test_log4cplus();
bool test_memory();
bool test_net();

int main() {
	printf("process: unit_test\n");

	if (!test_log4cplus()) {
		printf("%s", "测试日志系统失败");
	}

	if (false && !test_memory()) {
		printf("%s", "测试内存池工具失败");
	}

	if (!test_net()) {
		printf("%s", "测试网络模块错误");
	}

	return 0;
}

#include "memory/memory_dependency.h"
#include "memory/mem_define.h"

#include <stdio.h>
#include <string>

class TestClass {
	public:
		TestClass() {
			str_test = "this is a string test";
			int_test = 1024;
		}
		~TestClass() {
		}

		void PrintInfo() {
			printf("TestClass Info: %s | %d", str_test.c_str(), int_test);
		}

	private:
		std::string str_test;
		int int_test;
};

bool test_memory() {

	void* ptr = memory::nedpool_calloc_policy::allocateBytes(1024);;
	memory::nedpool_calloc_policy::deallocateBytes(ptr);

	go::vector<int>::type test_data;
	for (size_t i = 0; i < 1024; ++i) {
		test_data.push_back(i);
	}

	for (size_t i = 0; i < test_data.size(); ++i) {
		printf("%d_", test_data[i]);
	}

	do {
		
	} while (false);
	
	return false;
}

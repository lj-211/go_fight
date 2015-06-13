#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stdio.h>

namespace memory {

class ned_calloc_policy {
	public:
		static void* allocateBytes(size_t count, const char* file = NULL,
			int line = 0, const char* func = NULL);

		static void deallocateBytes(void* ptr);

		static size_t getMaxAllocationSize();
};

template <size_t Alignment = 0>
class ned_align_calloc_policy {
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment
			[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

		static void* allocateBytes(size_t count, const char* file = NULL,
			int line = 0, const char* func = NULL);

		static void deallocateBytes(void* ptr);

		static size_t getMaxAllocationSize();
};

class nedpool_calloc_policy {
	public:
		static void* allocateBytes(size_t count, const char* file = NULL,
			int line = 0, const char* func = NULL);

		static void deallocateBytes(void* ptr);

		static size_t getMaxAllocationSize();
};

template <size_t Alignment = 0>
class nedpool_align_calloc_policy {
	public:
		// compile-time check alignment is available.
		typedef int IsValidAlignment
			[Alignment <= 128 && ((Alignment & (Alignment-1)) == 0) ? +1 : -1];

		static void* allocateBytes(size_t count, const char* file = NULL,
			int line = 0, const char* func = NULL);

		static void deallocateBytes(void* ptr);

		static inline size_t getMaxAllocationSize();
};

} // end namespace memory

#endif

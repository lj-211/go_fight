#include "memory/mem_pool.h"

#include <stdlib.h>
#include <limits>

#include "nedmalloc/nedmalloc.c"

namespace {

const size_t s_default_align = 16;

// ---------------------------------------------------------------------------------------
// ned memory function
void* ned_alloc_bytes(size_t count, const char* file, int line, const char* func) {
	void* ptr = nedalloc::nedmalloc(count);
	return ptr;
}

void ned_dealloc_bytes(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	nedalloc::nedfree(ptr);
}

void* ned_alloc_bytes_align(size_t align, size_t count, const char* file, int line,
	const char* func) {

	void* ptr =  align ? nedalloc::nedmemalign(align, count)
		: nedalloc::nedmemalign(s_default_align, count);

	return ptr;

}

void ned_dealloc_bytes_align(size_t align, void* ptr) {
	if (ptr == NULL) {
		return;
	}

	nedalloc::nedfree(ptr);
}

// ----------------------------------------------------------------------------------------
// ned pool memory function

const size_t s_pool_count = 14;
void* s_pool_footprint = reinterpret_cast<void*>(0xBB1AA45A);
nedalloc::nedpool* s_pools[s_pool_count + 1] = {0};
nedalloc::nedpool* s_pools_aligned[s_pool_count + 1] = {0};

size_t size_match_id(size_t req_size) {
	size_t _pool_id = 0;

	if (req_size <= 16) {
		_pool_id = (req_size - 1) >> 2;
	} else {
		_pool_id = std::min<size_t>(((req_size - 1) >> 4) + 3, s_pool_count);
	}

	return _pool_id;
}

void* nedpool_alloc_bytes(size_t count, const char* file, int line, const char* func) {
	size_t _pool_id = size_match_id(count);
	nedalloc::nedpool* _pool = NULL;

	if (_pool_id < s_pool_count) {
		if (s_pools[_pool_id] == NULL) {
			s_pools[_pool_id] = nedalloc::nedcreatepool(0, 8);
			nedalloc::nedpsetvalue(s_pools[_pool_id], s_pool_footprint);
		}

		_pool = s_pools[_pool_id];
	}

	void* ptr = nedalloc::nedpmalloc(_pool, count);

	return ptr;
}

void nedpool_dealloc_bytes(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	nedalloc::nedpool* _pool = NULL;
	void* _foot_print = nedalloc::nedgetvalue(&_pool, ptr);
	if (_foot_print == s_pool_footprint) {
		nedalloc::nedpfree(_pool, ptr);
	} else {
		nedalloc::nedfree(ptr);
	}
}

void* nedpool_alloc_bytes_align(size_t align, size_t count, const char* file, int line,
	const char* func) {

	size_t _pool_id = size_match_id(count);
	nedalloc::nedpool* _pool = NULL;

	if (_pool_id < s_pool_count) {
		if (s_pools_aligned[_pool_id] == NULL) {
			s_pools_aligned[_pool_id] = nedalloc::nedcreatepool(0, 8);
			nedalloc::nedpsetvalue(s_pools_aligned[_pool_id], s_pool_footprint);
		}

		_pool = s_pools_aligned[_pool_id];
	}

	void* ptr = nedalloc::nedpmemalign(_pool, align, count);

	return ptr;
}

void nedpool_dealloc_bytes_align(size_t align, void* ptr) {
	nedpool_dealloc_bytes(ptr);
}

// --------------------------------------------------------------------------------------
// std memory funcion
void* std_alloc_bytes(size_t count, const char* file, int line, const char* func) {
	void* ptr = malloc(count);;
	return ptr;
}

void std_dealloc_bytes(void* ptr) {
	if (ptr == NULL) {
		return;
	}

	free(ptr);
}

void* std_alloc_bytes_align(size_t align, size_t count, const char* file, int line,
	const char* func) {
	return std_alloc_bytes(count, file, line, func);
}

void std_dealloc_bytes_align(size_t align, void* ptr) {
	return std_dealloc_bytes(ptr);
}

} // end namespace anonymous

namespace memory {

// ------------------------------------------------------------------------------
void* ned_calloc_policy::allocateBytes(size_t count, const char* file,
	int line, const char* func) {
	return ned_alloc_bytes(count, file, line, func);
}

void ned_calloc_policy::deallocateBytes(void* ptr) {
	ned_dealloc_bytes(ptr);
}

size_t ned_calloc_policy::getMaxAllocationSize() {
	return std::numeric_limits<size_t>::max();
}

// ------------------------------------------------------------------------------
template <size_t Alignment>
void* ned_align_calloc_policy<Alignment>::allocateBytes(size_t count, const char* file,
	int line, const char* func) {
	return ned_alloc_bytes_align(Alignment, count, file, line, func);
}

template <size_t Alignment>
void ned_align_calloc_policy<Alignment>::deallocateBytes(void* ptr) {
	ned_dealloc_bytes_align(Alignment, ptr);
}

template <size_t Alignment>
size_t ned_align_calloc_policy<Alignment>::getMaxAllocationSize() {
	return std::numeric_limits<size_t>::max();
}

// ------------------------------------------------------------------------------
void* nedpool_calloc_policy::allocateBytes(size_t count, const char* file,
	int line, const char* func) {
	return nedpool_alloc_bytes(count, file, line, func);
}

void nedpool_calloc_policy::deallocateBytes(void* ptr) {
	nedpool_dealloc_bytes(ptr);
}

size_t nedpool_calloc_policy::getMaxAllocationSize() {
	return std::numeric_limits<size_t>::max();
}

// ------------------------------------------------------------------------------
template <size_t Alignment>
void* nedpool_align_calloc_policy<Alignment>::allocateBytes(size_t count, const char* file,
	int line, const char* func) {
	return nedpool_alloc_bytes_align(Alignment, count, file, line, func);
}

template <size_t Alignment>
void nedpool_align_calloc_policy<Alignment>::deallocateBytes(void* ptr) {
	nedpool_dealloc_bytes_align(Alignment, ptr);
}

template <size_t Alignment>
size_t nedpool_align_calloc_policy<Alignment>::getMaxAllocationSize() {
	return std::numeric_limits<size_t>::max();
}

} // end namespace memory

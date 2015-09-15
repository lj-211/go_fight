#ifndef MEM_DEFINE_H
#define MEM_DEFINE_H

#include "memory/mem_pool.h"

#include <stdio.h>

#define USE_MEM_POOL 0
#define USE_MEM_POOL_ALIGN 0

template <typename T>
T* construct_arr(T* ptr, size_t count) {
        for (size_t i = 0; i < count; ++i)
        {
            new ((void*)(ptr+i)) T();
        }
        return ptr;
}

#if USE_MEM_POOL
#define GO_MALLOC(count) \
    memory::nedpool_calloc_policy::allocateBytes(count, __FILE__, __LINE__, __FUNCTION__)
#define GO_FREE(ptr) memory::nedpool_calloc_policy::deallocateBytes(ptr)

#define GO_NEW(T) \
    new (memory::nedpool_calloc_policy::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
#define GO_DELETE(ptr, T) \
    if (ptr) {(ptr)->~T(); memory::nedpool_calloc_policy::deallocateBytes(ptr);}

#define G_NEW_ARR(T, count) \
    construct_arr(static_cast<T*>(memory::nedpool_calloc_policy::allocateBytes(sizeof(T) * count, __FILE__, __LINE__, __FUNCTION__)), count)
#define G_DELETE_ARR(ptr, T, count) \
    if (ptr) { \
        for (size_t i = 0; i < count; ++i) { \
            (ptr)[i]->~T(); \
        } \
        memory::nedpool_calloc_policy::deallocateBytes(ptr); \
    } 

#elif USE_MEM_POOL_ALIGN
    #define DO_NOTHING
#else
#define GO_MALLOC(count) malloc(count)
#define GO_FREE(ptr) free(ptr)

#define GO_NEW(T) new T()
#define GO_DELETE(ptr, T) \
    if (ptr) { \
        delete ptr; \
    }

#define GO_NEW_ARR(T, count) new T[count]
#define GO_DELETE_ARR(ptr, T, count) \
    if (ptr) { \
        for (size_t i = 0; i < count; ++i) } \
            delete ptr[i]; \
        } \
    }
#endif

#define VECTOR(T) std::vector<T, memory::STLAllocator<T, memory::nedpool_calloc_policy> >
#define LIST(T) std::list<T, memory::STLAllocator<T, memory::nedpool_calloc_policy> >
#define MAP(K, V) std::map<K, V, std::less<K>, memory::STLAllocator<std::pair<const K, V>, memory::nedpool_calloc_policy> >
#define DEQUE(T) std::deque<T, memory::STLAllocator<T, memory::nedpool_calloc_policy> >


#endif

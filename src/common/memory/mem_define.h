#ifndef MEM_DEFINE_H
#define MEM_DEFINE_H

#include <stdio.h>
#include <vector>
#include <list>
#include <map>
#include <deque>

#define GO_MALLOC(count) malloc(count)
#define GO_FREE(ptr) free(ptr)

#define GO_NEW(T) new T()
#define GO_DELETE(ptr, T) \
    if (ptr) { \
        delete ptr; \
    }


#define VECTOR(T) std::vector<T, std::allocator<T> >
#define LIST(T) std::list<T, std::allocator<T> >
#define MAP(K, V) std::map<K, V, std::less<K>, std::allocator<std::pair<const K, V> > >
#define DEQUE(T) std::deque<T, std::allocator<T> >

#endif
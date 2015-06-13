#ifndef MEMORY_DEPENDENCY_H
#define MEMORY_DEPENDENCY_H

#include "memory/mem_define.h"
#include "memory/mempool_std_container.h"

#include "mem_pool.h"

#include <map>
#include <deque>
#include <vector>
#include <list>
#include <set>
#include <string>

#define USE_MEMPOOL_FOR_STD 1

namespace go {

#if USE_MEMPOOL_FOR_STD
	typedef std::basic_string<char, std::char_traits<char>, memory::STLAllocator<char, memory::nedpool_calloc_policy> >	_StringBase;
#else
	typedef std::string _StringBase;
#endif

#if USE_MEMPOOL_FOR_STD
	typedef std::basic_stringstream<char, std::char_traits<char>, memory::STLAllocator<char, memory::nedpool_calloc_policy> > _StringStreamBase;
#else
	typedef std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > _StringStreamBase;
#endif

	typedef _StringBase String;
	typedef _StringStreamBase StringStream;

// std container
template <typename T, typename A = memory::STLAllocator<T, memory::nedpool_calloc_policy> > 
	struct deque 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::deque<T, A> type;    
		typedef typename std::deque<T, A>::iterator iterator;
		typedef typename std::deque<T, A>::const_iterator const_iterator;
#else
		typedef typename std::deque<T> type;
		typedef typename std::deque<T>::iterator iterator;
		typedef typename std::deque<T>::const_iterator const_iterator;
#endif
	}; 

	template <typename T, typename A = memory::STLAllocator<T, memory::nedpool_calloc_policy> > 
	struct vector 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::vector<T, A> type;
		typedef typename std::vector<T, A>::iterator iterator;
		typedef typename std::vector<T, A>::const_iterator const_iterator;
#else
		typedef typename std::vector<T> type;
		typedef typename std::vector<T>::iterator iterator;
		typedef typename std::vector<T>::const_iterator const_iterator;
#endif
	}; 

	template <typename T, typename A = memory::STLAllocator<T, memory::nedpool_calloc_policy> > 
	struct list 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::list<T, A> type;
		typedef typename std::list<T, A>::iterator iterator;
		typedef typename std::list<T, A>::const_iterator const_iterator;
#else
		typedef typename std::list<T> type;
		typedef typename std::list<T>::iterator iterator;
		typedef typename std::list<T>::const_iterator const_iterator;
#endif
	}; 

	template <typename T, typename P = std::less<T>, typename A = memory::STLAllocator<T, memory::nedpool_calloc_policy> > 
	struct set 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::set<T, P, A> type;
		typedef typename std::set<T, P, A>::iterator iterator;
		typedef typename std::set<T, P, A>::const_iterator const_iterator;
#else
		typedef typename std::set<T, P> type;
		typedef typename std::set<T, P>::iterator iterator;
		typedef typename std::set<T, P>::const_iterator const_iterator;
#endif
	}; 

	template <typename K, typename V, typename P = std::less<K>, typename A = memory::STLAllocator<std::pair<const K, V>, memory::nedpool_calloc_policy> > 
	struct map 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::map<K, V, P, A> type;
		typedef typename std::map<K, V, P, A>::iterator iterator;
		typedef typename std::map<K, V, P, A>::const_iterator const_iterator;
#else
		typedef typename std::map<K, V, P> type;
		typedef typename std::map<K, V, P>::iterator iterator;
		typedef typename std::map<K, V, P>::const_iterator const_iterator;
#endif
	}; 

	template <typename K, typename V, typename P = std::less<K>, typename A = memory::STLAllocator<std::pair<const K, V>, memory::nedpool_calloc_policy> > 
	struct multimap 
	{ 
#if USE_MEMPOOL_FOR_STD
		typedef typename std::multimap<K, V, P, A> type;
		typedef typename std::multimap<K, V, P, A>::iterator iterator;
		typedef typename std::multimap<K, V, P, A>::const_iterator const_iterator;
#else
		typedef typename std::multimap<K, V, P> type;
		typedef typename std::multimap<K, V, P>::iterator iterator;
		typedef typename std::multimap<K, V, P>::const_iterator const_iterator;
#endif
	}; 

} // end namespace GO

#endif

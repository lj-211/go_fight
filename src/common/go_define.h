#ifndef GO_DEFINE_H
#define G0_DEFINE_H

#include "memory/mem_define.h"
#include "memory/memory_dependency.h"

#define DISALLOW_COPY_AND_ASSIGN(T) \
	T(const T&); \
	void operator=(const T&)

typedef void (*log_error)(const char*);
typedef void (*log_trace)(const char*);

typedef void (*log_error_raw)(const char*, int, const char*, ...);
typedef void (*log_trace_raw)(const char*, int, const char*, ...);

#endif

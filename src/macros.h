#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define LOG(f, p, s, ...)												\
	fprintf(f, "[\033[1m%s\033[0m] [%s:%d in %s()]: ", p, __FILE__, __LINE__, __FUNCTION__);			\
	fprintf(f, s "\n", ##__VA_ARGS__)

#define __LOG_INFO_PREFIX	"\033[34mINFO "
#define __LOG_WARN_PREFIX	"\033[33mWARN "
#define __LOG_ERROR_PREFIX	"\033[31mERROR"

#define INFO(...)	LOG(stdout, __LOG_INFO_PREFIX,	__VA_ARGS__)
#define WARN(...)	LOG(stdout, __LOG_WARN_PREFIX,	__VA_ARGS__)
#define ERROR(...)	LOG(stderr, __LOG_ERROR_PREFIX,	__VA_ARGS__)

#define ASSERT(e, ...)	{												\
	if (!(e)) {													\
		ERROR("assertion '" #e "' failed: " __VA_ARGS__);							\
		abort();												\
	}														\
}

#define R_NOT_NULL(v, ...) ({												\
	void *_v;													\
	_v = v;														\
	ASSERT(_v != NULL, __VA_ARGS__);										\
	_v;														\
})

#define STAT(f) ({													\
	struct stat *_s = alloca(sizeof(struct stat));									\
	stat(f, _s) != 0 ? NULL : _s;											\
})

#define DEG_TO_RAD(r) ((r) * M_PI / 180)

#ifndef _DEBUG

#define __LOG_DEBUG_PREFIX	""
#define DEBUG(...)
#define DEBUG_ONLY(...)

#else

#define __LOG_DEBUG_PREFIX	"\033[0m\033[2mDEBUG"

#define DEBUG(...)	LOG(stderr, __LOG_DEBUG_PREFIX, __VA_ARGS__)
#define DEBUG_ONLY(...)	{ __VA_ARGS__ }

#endif /** _DEBUG */

#endif /** MACROS_H */

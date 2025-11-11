#ifndef _LOG_H_
#define _LOG_H_

#include <sqlite3.h>

#define WHERESTR "[%28s][%25s] <<%38s>>, line %i: "
#define WHEREARG  __FILE__,__func__,__LINE__

// #define PRINT_LOG

#ifdef PRINT_LOG
#define codbg(fmt, arg...) \
	do { \
		printf(""fmt"\n", ##arg);	\
	} while (0)
#else
#define codbg(...) codb_dprintf_impl(WHEREARG, __VA_ARGS__)
#endif

#define LIBCODB_DEBUG_TO_FILE "/tmp/LIBCODB_DEBUG_FILE"

void codb_dprintf_impl(const char* file, const char* func, size_t line, const char* fmt, ...);
#endif
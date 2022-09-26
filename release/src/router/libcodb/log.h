#ifndef _LOG_H_
#define _LOG_H_

#include <sqlite3.h>

#define WHERESTR "%s, %i: "
#define WHEREARG  __func__,__LINE__
#define codbg(pdb, ...) dprintf_impl(WHEREARG, pdb, __VA_ARGS__)

#define LIBCODB_DEBUG_TO_FILE "/tmp/LIBCODB_DEBUG_FILE"

void dprintf_impl(const char* func, size_t line, sqlite3 *pdb, const char* fmt, ...);

#endif
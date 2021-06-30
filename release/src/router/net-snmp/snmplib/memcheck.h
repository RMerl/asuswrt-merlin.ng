#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#define MAKE_MEM_DEFINED(ptr, len) VALGRIND_MAKE_MEM_DEFINED(ptr, len)
#else
#define MAKE_MEM_DEFINED(ptr, len) do { } while (0)
#endif

#endif /* _MEMCHECK_H_ */

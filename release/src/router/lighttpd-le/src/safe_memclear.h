#ifndef _SAFE_MEMCLEAR_H_
#define _SAFE_MEMCLEAR_H_
#include "first.h"

/* size_t */
#include <sys/types.h>

void safe_memclear(void *s, size_t n);

#endif

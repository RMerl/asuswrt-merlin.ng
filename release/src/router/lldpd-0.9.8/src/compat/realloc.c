/* -*- mode: c; c-file-style: "openbsd" -*- */
/* realloc replacement that can reallocate 0 byte or NULL pointers*/

#undef realloc
#include <stdlib.h>
#include <sys/types.h>

/* Reallocate an N-byte block of memory from the heap.
   If N is zero, allocate a 1-byte block.  */
void *
rpl_realloc(void *ptr, size_t n)
{
	if (!ptr) return malloc(n);
	if (n == 0) n = 1;
	return realloc(ptr, n);
}

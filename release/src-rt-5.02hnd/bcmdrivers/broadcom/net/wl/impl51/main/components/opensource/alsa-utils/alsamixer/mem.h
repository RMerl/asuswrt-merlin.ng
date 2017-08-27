#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include <stddef.h>

void *ccalloc(size_t n, size_t size);
void *crealloc(void *ptr, size_t new_size);
char *cstrdup(const char *s);
char *casprintf(const char *fmt, ...) __attribute__((__format__(printf, 1, 2)));

#endif

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdlib.h>

struct vector;

struct vector *vector_create(size_t size);
void vector_destroy(struct vector *v);
int vector_add(struct vector *v, void *data);
int vector_iterate(struct vector *v, const void *data, int (*fcn)(const void *a, const void *b));

#endif

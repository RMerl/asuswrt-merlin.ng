#include "first.h"

#include "vector.h"
#include "base.h"

void *vector_realloc(void *data, size_t elem_size, size_t size, size_t used) {
	const size_t total_size = elem_size * size;
	const size_t used_size = elem_size * used;
	force_assert(size <= SIZE_MAX / elem_size);
	data = realloc(data, total_size);
	force_assert(NULL != data);

	/* clear new memory */
	memset(((char*)data) + used_size, 0, total_size - used_size);

	return data;
}

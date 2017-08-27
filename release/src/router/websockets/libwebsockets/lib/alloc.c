#include "private-libwebsockets.h"

static void *_realloc(void *ptr, size_t size)
{
	if (size)
		return realloc(ptr, size);
	else if (ptr)
		free(ptr);
	return NULL;
}

void *(*_lws_realloc)(void *ptr, size_t size) = _realloc;

void *lws_realloc(void *ptr, size_t size)
{
	return _lws_realloc(ptr, size);
}

void *lws_zalloc(size_t size)
{
	void *ptr = _lws_realloc(NULL, size);
	if (ptr)
		memset(ptr, 0, size);
	return ptr;
}

void lws_set_allocator(void *(*cb)(void *ptr, size_t size))
{
	_lws_realloc = cb;
}

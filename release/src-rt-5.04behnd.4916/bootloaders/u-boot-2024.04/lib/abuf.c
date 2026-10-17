// SPDX-License-Identifier: GPL-2.0+
/*
 * Handles a buffer that can be allocated and freed
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef USE_HOSTCC
#include <malloc.h>
#include <mapmem.h>
#include <string.h>
#endif

#include <abuf.h>

void abuf_set(struct abuf *abuf, void *data, size_t size)
{
	abuf_uninit(abuf);
	abuf->data = data;
	abuf->size = size;
}

#ifndef USE_HOSTCC
void abuf_map_sysmem(struct abuf *abuf, ulong addr, size_t size)
{
	abuf_set(abuf, map_sysmem(addr, size), size);
}
#else
/* copied from lib/string.c for convenience */
static char *memdup(const void *src, size_t len)
{
	char *p;

	p = malloc(len);
	if (!p)
		return NULL;

	memcpy(p, src, len);

	return p;
}
#endif

bool abuf_realloc(struct abuf *abuf, size_t new_size)
{
	void *ptr;

	if (!new_size) {
		/* easy case, just need to uninit, freeing any allocation */
		abuf_uninit(abuf);
		return true;
	} else if (abuf->alloced) {
		/* currently allocated, so need to reallocate */
		ptr = realloc(abuf->data, new_size);
		if (!ptr)
			return false;
		abuf->data = ptr;
		abuf->size = new_size;
		return true;
	} else if (new_size <= abuf->size) {
		/*
		 * not currently alloced and new size is no larger. Just update
		 * it. Data is lost off the end if new_size < abuf->size
		 */
		abuf->size = new_size;
		return true;
	} else {
		/* not currently allocated and new size is larger. Alloc and
		 * copy in data. The new space is not inited.
		 */
		ptr = malloc(new_size);
		if (!ptr)
			return false;
		if (abuf->size)
			memcpy(ptr, abuf->data, abuf->size);
		abuf->data = ptr;
		abuf->size = new_size;
		abuf->alloced = true;
		return true;
	}
}

bool abuf_realloc_inc(struct abuf *abuf, size_t inc)
{
	return abuf_realloc(abuf, abuf->size + inc);
}

void *abuf_uninit_move(struct abuf *abuf, size_t *sizep)
{
	void *ptr;

	if (sizep)
		*sizep = abuf->size;
	if (!abuf->size)
		return NULL;
	if (abuf->alloced) {
		ptr = abuf->data;
	} else {
		ptr = memdup(abuf->data, abuf->size);
		if (!ptr)
			return NULL;
	}
	/* Clear everything out so there is no record of the data */
	abuf_init(abuf);

	return ptr;
}

void abuf_init_set(struct abuf *abuf, void *data, size_t size)
{
	abuf_init(abuf);
	abuf_set(abuf, data, size);
}

void abuf_init_move(struct abuf *abuf, void *data, size_t size)
{
	abuf_init_set(abuf, data, size);
	abuf->alloced = true;
}

void abuf_uninit(struct abuf *abuf)
{
	if (abuf->alloced)
		free(abuf->data);
	abuf_init(abuf);
}

void abuf_init(struct abuf *abuf)
{
	abuf->data = NULL;
	abuf->size = 0;
	abuf->alloced = false;
}

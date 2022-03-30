// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#include <common.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static void *alloc_simple(size_t bytes, int align)
{
	ulong addr, new_ptr;
	void *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%zx, ptr=%lx, limit=%lx: ", bytes, new_ptr,
		  gd->malloc_limit);
	if (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted\n");
		return NULL;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, sizeof(new_ptr));

	return ptr;
}

void *malloc_simple(size_t bytes)
{
	void *ptr;

	ptr = alloc_simple(bytes, 1);
	if (!ptr)
		return ptr;

	log_debug("%lx\n", (ulong)ptr);

	return ptr;
}

void *memalign_simple(size_t align, size_t bytes)
{
	void *ptr;

	ptr = alloc_simple(bytes, align);
	if (!ptr)
		return ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);

	return ptr;
}

#if CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
void *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	void *ptr;

	ptr = malloc(size);
	if (!ptr)
		return ptr;
	memset(ptr, '\0', size);

	return ptr;
}
#endif

void malloc_simple_info(void)
{
	log_info("malloc_simple: %lx bytes used, %lx remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}

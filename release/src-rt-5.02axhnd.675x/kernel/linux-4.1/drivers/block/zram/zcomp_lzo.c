/*
 * Copyright (C) 2014 Sergey Senozhatsky.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/lzo.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

#include "zcomp_lzo.h"

static void *lzo_create(void)
{
	void *ret;

	/*
	 * This function can be called in swapout/fs write path
	 * so we can't use GFP_FS|IO. And it assumes we already
	 * have at least one stream in zram initialization so we
	 * don't do best effort to allocate more stream in here.
	 * A default stream will work well without further multiple
	 * streams. That's why we use NORETRY | NOWARN.
	 */
	ret = kzalloc(LZO1X_MEM_COMPRESS, GFP_NOIO | __GFP_NORETRY |
					__GFP_NOWARN);
	if (!ret)
		ret = __vmalloc(LZO1X_MEM_COMPRESS,
				GFP_NOIO | __GFP_NORETRY | __GFP_NOWARN |
				__GFP_ZERO | __GFP_HIGHMEM,
				PAGE_KERNEL);
	return ret;
}

static void lzo_destroy(void *private)
{
	kvfree(private);
}

static int lzo_compress(const unsigned char *src, unsigned char *dst,
		size_t *dst_len, void *private)
{
	int ret = lzo1x_1_compress(src, PAGE_SIZE, dst, dst_len, private);
	return ret == LZO_E_OK ? 0 : ret;
}

static int lzo_decompress(const unsigned char *src, size_t src_len,
		unsigned char *dst)
{
	size_t dst_len = PAGE_SIZE;
	int ret = lzo1x_decompress_safe(src, src_len, dst, &dst_len);
	return ret == LZO_E_OK ? 0 : ret;
}

struct zcomp_backend zcomp_lzo = {
	.compress = lzo_compress,
	.decompress = lzo_decompress,
	.create = lzo_create,
	.destroy = lzo_destroy,
	.name = "lzo",
};

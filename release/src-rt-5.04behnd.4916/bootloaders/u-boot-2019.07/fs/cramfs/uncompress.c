/*
 * uncompress.c
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 2000-2002 Transmeta Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * cramfs interfaces to the uncompression library. There's really just
 * three entrypoints:
 *
 *  - cramfs_uncompress_init() - called to initialize the thing.
 *  - cramfs_uncompress_exit() - tell me when you're done
 *  - cramfs_uncompress_block() - uncompress a block.
 *
 * NOTE NOTE NOTE! The uncompression is entirely single-threaded. We
 * only have one stream, and we'll initialize it only once even if it
 * then is used by multiple filesystems.
 */

#include <common.h>
#include <malloc.h>
#include <watchdog.h>
#include <u-boot/zlib.h>

static z_stream stream;

/* Returns length of decompressed data. */
int cramfs_uncompress_block (void *dst, void *src, int srclen)
{
	int err;

	inflateReset (&stream);

	stream.next_in = src;
	stream.avail_in = srclen;

	stream.next_out = dst;
	stream.avail_out = 4096 * 2;

	err = inflate (&stream, Z_FINISH);

	if (err != Z_STREAM_END)
		goto err;
	return stream.total_out;

      err:
	/*printf ("Error %d while decompressing!\n", err); */
	/*printf ("%p(%d)->%p\n", src, srclen, dst); */
	return -1;
}

int cramfs_uncompress_init (void)
{
	int err;

	stream.zalloc = gzalloc;
	stream.zfree = gzfree;
	stream.next_in = 0;
	stream.avail_in = 0;

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	stream.outcb = (cb_func) WATCHDOG_RESET;
#else
	stream.outcb = Z_NULL;
#endif /* CONFIG_HW_WATCHDOG */

	err = inflateInit (&stream);
	if (err != Z_OK) {
		printf ("Error: inflateInit2() returned %d\n", err);
		return -1;
	}

	return 0;
}

int cramfs_uncompress_exit (void)
{
	inflateEnd (&stream);
	return 0;
}

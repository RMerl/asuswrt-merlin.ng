// SPDX-License-Identifier: GPL-2.0+
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 */
/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <common.h>
#include <malloc.h>
#include <linux/stat.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include "zfs_common.h"

#include <zfs/zfs.h>
#include <zfs/zio.h>
#include <zfs/dnode.h>
#include <zfs/uberblock_impl.h>
#include <zfs/vdev_impl.h>
#include <zfs/zio_checksum.h>
#include <zfs/zap_impl.h>
#include <zfs/zap_leaf.h>
#include <zfs/zfs_znode.h>
#include <zfs/dmu.h>
#include <zfs/dmu_objset.h>
#include <zfs/dsl_dir.h>
#include <zfs/dsl_dataset.h>

#define	MATCH_BITS	6
#define	MATCH_MIN	3
#define	OFFSET_MASK	((1 << (16 - MATCH_BITS)) - 1)

/*
 * Decompression Entry - lzjb
 */
#ifndef	NBBY
#define	NBBY	8
#endif

int
lzjb_decompress(void *s_start, void *d_start, uint32_t s_len,
				uint32_t d_len)
{
	uint8_t *src = s_start;
	uint8_t *dst = d_start;
	uint8_t *d_end = (uint8_t *) d_start + d_len;
	uint8_t *s_end = (uint8_t *) s_start + s_len;
	uint8_t *cpy, copymap = 0;
	int copymask = 1 << (NBBY - 1);

	while (dst < d_end && src < s_end) {
		if ((copymask <<= 1) == (1 << NBBY)) {
			copymask = 1;
			copymap = *src++;
		}
		if (src >= s_end) {
			printf("lzjb decompression failed\n");
			return ZFS_ERR_BAD_FS;
		}
		if (copymap & copymask) {
			int mlen = (src[0] >> (NBBY - MATCH_BITS)) + MATCH_MIN;
			int offset = ((src[0] << NBBY) | src[1]) & OFFSET_MASK;
			src += 2;
			cpy = dst - offset;
			if (src > s_end || cpy < (uint8_t *) d_start) {
				printf("lzjb decompression failed\n");
				return ZFS_ERR_BAD_FS;
			}
			while (--mlen >= 0 && dst < d_end)
				*dst++ = *cpy++;
		} else {
			*dst++ = *src++;
		}
	}
	if (dst < d_end) {
		printf("lzjb decompression failed\n");
		return ZFS_ERR_BAD_FS;
	}
	return ZFS_ERR_NONE;
}

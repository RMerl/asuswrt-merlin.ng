/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#ifndef SQFS_DECOMPRESSOR_H
#define SQFS_DECOMPRESSOR_H

#include <stdint.h>
#include "sqfs_filesystem.h"

#define SQFS_COMP_ZLIB 1
#define SQFS_COMP_LZMA 2
#define SQFS_COMP_LZO 3
#define SQFS_COMP_XZ 4
#define SQFS_COMP_LZ4 5
#define SQFS_COMP_ZSTD 6

int sqfs_decompress(struct squashfs_ctxt *ctxt, void *dest,
		    unsigned long *dest_len, void *source, u32 src_len);
int sqfs_decompressor_init(struct squashfs_ctxt *ctxt);
void sqfs_decompressor_cleanup(struct squashfs_ctxt *ctxt);

#endif /* SQFS_DECOMPRESSOR_H */

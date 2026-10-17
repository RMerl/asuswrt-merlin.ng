/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#ifndef SQFS_UTILS_H
#define SQFS_UTILS_H

#include <linux/bitops.h>
#include <linux/kernel.h>
#include <stdbool.h>

#define SQFS_FRAGMENT_INDEX_OFFSET(A) ((A) % SQFS_MAX_ENTRIES)
#define SQFS_FRAGMENT_INDEX(A) ((A) / SQFS_MAX_ENTRIES)
#define SQFS_BLOCK_SIZE(A) ((A) & GENMASK(23, 0))
/* Useful for both fragment and data blocks */
#define SQFS_COMPRESSED_BLOCK(A) (!((A) & BIT(24)))
#define SQFS_IS_FRAGMENTED(A) ((A) != 0xFFFFFFFF)
/*
 * These two macros work as getters for a metada block header, retrieving the
 * data size and if it is compressed/uncompressed
 */
#define SQFS_COMPRESSED_METADATA(A) (!((A) & BIT(15)))
#define SQFS_METADATA_SIZE(A) ((A) & GENMASK(14, 0))

#endif /* SQFS_UTILS_H  */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 */

#ifndef _SHA_H
#define _SHA_H

#include <fsl_sec.h>
#include <hash.h>
#include "jr.h"

/* We support at most 32 Scatter/Gather Entries.*/
#define MAX_SG_32	32

/*
 * Hash context contains the following fields
 * @sha_desc: Sha Descriptor
 * @sg_num: number of entries in sg table
 * @len: total length of buffer
 * @sg_tbl: sg entry table
 * @hash: index to the hash calculated
 */
struct sha_ctx {
	uint32_t sha_desc[64];
	uint32_t sg_num;
	uint32_t len;
	struct sg_entry sg_tbl[MAX_SG_32];
	u8 hash[HASH_MAX_DIGEST_SIZE];
};

#endif

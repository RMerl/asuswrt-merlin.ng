/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 General Electric Company
 */

#include "common.h"

struct vpd_cache;

/*
 * Read VPD from given data, verify content, call callback for each vital
 * product data block.
 *
 * cache: structure used by process block to store VPD information
 * process_block: callback called for each VPD data block
 *
 * Returns Non-zero on error.  Negative numbers encode errno.
 */
int read_vpd(struct vpd_cache *cache,
	     int (*process_block)(struct vpd_cache *,
				  u8 id, u8 version, u8 type,
				  size_t size, u8 const *data));

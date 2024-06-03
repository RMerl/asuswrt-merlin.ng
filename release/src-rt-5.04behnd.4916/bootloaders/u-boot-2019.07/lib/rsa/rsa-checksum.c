// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Andreas Oetken.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <fdtdec.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <hash.h>
#else
#include "fdt_host.h"
#endif
#include <u-boot/rsa.h>

int hash_calculate(const char *name,
		    const struct image_region region[],
		    int region_count, uint8_t *checksum)
{
	struct hash_algo *algo;
	int ret = 0;
	void *ctx;
	uint32_t i;
	i = 0;

	ret = hash_progressive_lookup_algo(name, &algo);
	if (ret)
		return ret;

	ret = algo->hash_init(algo, &ctx);
	if (ret)
		return ret;

	for (i = 0; i < region_count - 1; i++) {
		ret = algo->hash_update(algo, ctx, region[i].data,
					region[i].size, 0);
		if (ret)
			return ret;
	}

	ret = algo->hash_update(algo, ctx, region[i].data, region[i].size, 1);
	if (ret)
		return ret;
	ret = algo->hash_finish(algo, ctx, checksum, algo->digest_size);
	if (ret)
		return ret;

	return 0;
}

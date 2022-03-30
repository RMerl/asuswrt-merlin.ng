// SPDX-License-Identifier: GPL-2.0+
/*
 * Miscellaneous Snapdragon functionality
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include <common.h>
#include <mmc.h>
#include <asm/arch/misc.h>

/* UNSTUFF_BITS macro taken from Linux Kernel: drivers/mmc/core/sd.c */
#define UNSTUFF_BITS(resp, start, size) \
	({ \
		const int __size = size; \
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32); \
		const int __shft = (start) & 31; \
		u32 __res; \
					\
		__res = resp[__off] >> __shft; \
		if (__size + __shft > 32) \
			__res |= resp[__off - 1] << ((32 - __shft) % 32); \
		__res & __mask;	\
	})

u32 msm_board_serial(void)
{
	struct mmc *mmc_dev;

	mmc_dev = find_mmc_device(0);
	if (!mmc_dev)
		return 0;

	return UNSTUFF_BITS(mmc_dev->cid, 16, 32);
}

void msm_generate_mac_addr(u8 *mac)
{
	int i;
	char sn[9];

	snprintf(sn, 8, "%08x", msm_board_serial());

	/* fill in the mac with serialno, use locally adminstrated pool */
	mac[0] = 0x02;
	mac[1] = 00;
	for (i = 3; i >= 0; i--) {
		mac[i + 2] = simple_strtoul(&sn[2 * i], NULL, 16);
		sn[2 * i] = 0;
	}
}

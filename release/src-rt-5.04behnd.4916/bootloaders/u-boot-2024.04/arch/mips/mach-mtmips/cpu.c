// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <event.h>
#include <init.h>
#include <malloc.h>
#include <asm/addrspace.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)KSEG1, CONFIG_MAX_MEM_SIZE << 20);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int last_stage_init(void)
{
	void *src, *dst;

	src = malloc(SZ_64K);
	dst = malloc(SZ_64K);
	if (!src || !dst) {
		printf("Can't allocate buffer for cache cleanup copy!\n");
		return 0;
	}

	/*
	 * It has been noticed, that sometimes the d-cache is not in a
	 * "clean-state" when U-Boot is running on MT7688. This was
	 * detected when using the ethernet driver (which uses d-cache)
	 * and a TFTP command does not complete. Copying an area of 64KiB
	 * in DDR at a very late bootup time in U-Boot, directly before
	 * calling into the prompt, seems to fix this issue.
	 */
	memcpy(dst, src, SZ_64K);
	free(src);
	free(dst);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);
#endif

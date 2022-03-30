// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: R5 MPU region definitions
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include "common.h"

struct mpu_region_config k3_mpu_regions[16] = {
	/*
	 * Make all 4GB as Device Memory and not executable. We are overriding
	 * it with next region for any requirement.
	 */
	{0x00000000, REGION_0, XN_EN, PRIV_RW_USR_RW, SHARED_WRITE_BUFFERED,
	 REGION_4GB},

	/* SPL code area marking it as WB and Write allocate. */
	{CONFIG_SPL_TEXT_BASE, REGION_1, XN_DIS, PRIV_RW_USR_RW,
	 O_I_WB_RD_WR_ALLOC, REGION_8MB},

	/* U-Boot's code area marking it as WB and Write allocate */
	{CONFIG_SYS_SDRAM_BASE, REGION_2, XN_DIS, PRIV_RW_USR_RW,
	 O_I_WB_RD_WR_ALLOC, REGION_2GB},
	{0x0, 3, 0x0, 0x0, 0x0, 0x0},
	{0x0, 4, 0x0, 0x0, 0x0, 0x0},
	{0x0, 5, 0x0, 0x0, 0x0, 0x0},
	{0x0, 6, 0x0, 0x0, 0x0, 0x0},
	{0x0, 7, 0x0, 0x0, 0x0, 0x0},
	{0x0, 8, 0x0, 0x0, 0x0, 0x0},
	{0x0, 9, 0x0, 0x0, 0x0, 0x0},
	{0x0, 10, 0x0, 0x0, 0x0, 0x0},
	{0x0, 11, 0x0, 0x0, 0x0, 0x0},
	{0x0, 12, 0x0, 0x0, 0x0, 0x0},
	{0x0, 13, 0x0, 0x0, 0x0, 0x0},
	{0x0, 14, 0x0, 0x0, 0x0, 0x0},
	{0x0, 15, 0x0, 0x0, 0x0, 0x0},
};

void setup_k3_mpu_regions(void)
{
	setup_mpu_regions(k3_mpu_regions, ARRAY_SIZE(k3_mpu_regions));
}

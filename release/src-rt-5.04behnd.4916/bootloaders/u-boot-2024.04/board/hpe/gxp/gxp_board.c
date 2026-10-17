// SPDX-License-Identifier: GPL-2.0+
/*
 * GXP timer driver
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 */

#include <linux/sizes.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass.h>
#include <ram.h>

DECLARE_GLOBAL_DATA_PTR;

#define ECHI_CMD 0xcefe0010

int board_init(void)
{
	writel(0x00080002, ECHI_CMD);

	return 0;
}

int dram_init(void)
{
	if (IS_ENABLED(CONFIG_TARGET_GXP)) {
		if (IS_ENABLED(CONFIG_GXP_ECC)) {
			/* 0x0f800000 */
			gd->ram_size = SZ_128M + SZ_64M + SZ_32M + SZ_16M + SZ_8M;
		} else {
			/* 0x1f000000 */
			gd->ram_size = SZ_256M + SZ_128M + SZ_64M + SZ_32M + SZ_16M;
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_64MB)) {
			if (IS_ENABLED(CONFIG_GXP_ECC)) {
				/* 0x0c000000 */
				gd->ram_size = SZ_128M + SZ_64M;
			} else {
				/* 0x18000000 */
				gd->ram_size = SZ_256M + SZ_128M;
			}
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_32MB)) {
			if (IS_ENABLED(CONFIG_GXP_ECC)) {
				/* 0x0e000000 */
				gd->ram_size = SZ_128M + SZ_64M + SZ_32M;
			} else {
				/* 0x1c000000 */
				gd->ram_size = SZ_256M + SZ_128M + SZ_64M;
			}
		}
	}

	if (IS_ENABLED(CONFIG_TARGET_GXP2)) {
		/* 0x1b200000 */
		gd->ram_size = SZ_256M + SZ_128M + SZ_32M + SZ_16M + SZ_2M;
		if (IS_ENABLED(CONFIG_GXP_VROM_64MB)) {
			/* 0x14000000 */
			gd->ram_size = SZ_256M + SZ_64M;
		}

		if (IS_ENABLED(CONFIG_GXP_VROM_32MB)) {
			/* 0x18000000 */
			gd->ram_size = SZ_256M + SZ_128M;
		}
	}

	return 0;
}


// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int mmc_get_boot_dev(void)
{
	int g_mmc_devid = -1;
	char *uflag = (char *)0x81DFFFF0;
	if (strncmp(uflag,"eMMC",4)==0) {
		g_mmc_devid = 0;
		printf("Boot From Emmc(id:%d)\n\n", g_mmc_devid);
	} else {
		g_mmc_devid = 1;
		printf("Boot From SD(id:%d)\n\n", g_mmc_devid);
	}
	return g_mmc_devid;
}

int mmc_get_env_dev(void)
{
	return mmc_get_boot_dev();
}

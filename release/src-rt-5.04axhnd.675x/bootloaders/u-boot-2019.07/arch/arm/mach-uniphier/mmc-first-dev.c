// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <mmc.h>
#include <linux/errno.h>

static int find_first_mmc_device(void)
{
	struct mmc *mmc;
	int i;

	for (i = 0; (mmc = find_mmc_device(i)); i++) {
		if (!mmc_init(mmc) && IS_MMC(mmc))
			return i;
	}

	return -ENODEV;
}

int mmc_get_env_dev(void)
{
	return find_first_mmc_device();
}

static int do_mmcsetn(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev;

	dev = find_first_mmc_device();
	if (dev < 0)
		return CMD_RET_FAILURE;

	env_set_ulong("mmc_first_dev", dev);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	   mmcsetn,	1,	1,	do_mmcsetn,
	"Set the first MMC (not SD) dev number to \"mmc_first_dev\" environment",
	""
);

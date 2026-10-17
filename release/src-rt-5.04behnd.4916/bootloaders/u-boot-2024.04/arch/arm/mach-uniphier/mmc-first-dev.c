// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <command.h>
#include <env.h>
#include <mmc.h>
#include <linux/errno.h>

static int find_first_mmc_device(bool is_sd)
{
	struct mmc *mmc;
	int i;

	for (i = 0; (mmc = find_mmc_device(i)); i++) {
		if (!mmc_init(mmc) &&
		    ((is_sd && IS_SD(mmc)) || (!is_sd && IS_MMC(mmc))))
			return i;
	}

	return -ENODEV;
}

int mmc_get_env_dev(void)
{
	return find_first_mmc_device(false);
}

static int do_mmcsetn(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int dev;

	dev = find_first_mmc_device(false);
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

static int do_sdsetn(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int dev;

	dev = find_first_mmc_device(true);
	if (dev < 0)
		return CMD_RET_FAILURE;

	env_set_ulong("sd_first_dev", dev);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	sdsetn,	1,	1,	do_sdsetn,
	"Set the first SD dev number to \"sd_first_dev\" environment",
	""
);

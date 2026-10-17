// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Copyright (C) 2020 Engicam S.r.l.
 * Copyright (C) 2020 Amarula Solutions(India)
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <power/regulator.h>

int checkboard(void)
{
	char *mode;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_TFABOOT))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = ofnode_get_property(ofnode_root(), "compatible",
					 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	return 0;
}

/* board dependent setup after realloc */
int board_init(void)
{
	if (IS_ENABLED(CONFIG_DM_REGULATOR))
		regulators_enable_boot_on(_DEBUG);

	return 0;
}

int board_late_init(void)
{
	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
		else if (CONFIG_IS_ENABLED(ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NAND:
	case BOOT_FLASH_SPINAND:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_UBI))
			return ENVL_UBI;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NOR:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		else
			return ENVL_NOWHERE;

	default:
		return ENVL_NOWHERE;
	}
}

const char *env_ext4_get_intf(void)
{
	u32 bootmode = get_bootmode();

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		return "mmc";
	default:
		return "";
	}
}

const char *env_ext4_get_dev_part(void)
{
	static char *const dev_part[] = {"0:auto", "1:auto", "2:auto"};
	u32 bootmode = get_bootmode();

	return dev_part[(bootmode & TAMP_BOOT_INSTANCE_MASK) - 1];
}

int mmc_get_env_dev(void)
{
	u32 bootmode = get_bootmode();

	return (bootmode & TAMP_BOOT_INSTANCE_MASK) - 1;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

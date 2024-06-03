// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch-rockchip/hardware.h>

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

#define SGRF_DDR_CON0 0x10150000
void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret) {
		printf("spl_early_init() failed: %d\n", ret);
		hang();
	}
	preloader_console_init();

	/* Disable the ddr secure region setting to make it non-secure */
	rk_clrreg(SGRF_DDR_CON0, 0x4000);
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

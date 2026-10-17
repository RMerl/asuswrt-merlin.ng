// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <image.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <led.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_PCB116 = 0xAABBCE00,
};

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE;

	return 0;
}

static void do_board_detect(void)
{
	gd->board_type = BOARD_TYPE_PCB116; /* ServalT */
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB116 &&
	    strcmp(name, "servalt_pcb116") == 0)
		return 0;
	return -1;
}
#endif

#if defined(CONFIG_DTB_RESELECT)
int embedded_dtb_select(void)
{
	do_board_detect();
	fdtdec_setup();

	return 0;
}
#endif

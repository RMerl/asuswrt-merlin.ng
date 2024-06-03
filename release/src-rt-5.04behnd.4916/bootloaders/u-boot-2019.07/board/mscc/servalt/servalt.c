// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <led.h>

enum {
	BOARD_TYPE_PCB116 = 0xAABBCE00,
};

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE;

	/* LED setup */
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

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

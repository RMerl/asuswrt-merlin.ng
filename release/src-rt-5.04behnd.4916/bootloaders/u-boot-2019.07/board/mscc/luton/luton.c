// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <led.h>
#include <miiphy.h>

enum {
	BOARD_TYPE_PCB090 = 0xAABBCD00,
	BOARD_TYPE_PCB091,
};

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	mscc_gpio_set_alternate(30, 1);
	mscc_gpio_set_alternate(31, 1);
}

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

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, 0, 31, 0x10);
	phy_write(phydev, 0, 18, 0x80A0);
	while (phy_read(phydev, 0, 18) & 0x8000)
		;
	phy_write(phydev, 0, 31, 0);
	return 0;
}

static void do_board_detect(void)
{
	u32 chipid = (readl(BASE_DEVCPU_GCB + CHIP_ID) >> 12) & 0xFFFF;

	if (chipid == 0x7428 || chipid == 0x7424)
		gd->board_type = BOARD_TYPE_PCB091;    // Lu10
	else
		gd->board_type = BOARD_TYPE_PCB090;    // Lu26
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB090 &&
	    strcmp(name, "luton_pcb090") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB091 &&
	    strcmp(name, "luton_pcb091") == 0)
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

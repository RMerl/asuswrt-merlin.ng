// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <led.h>
#include <miiphy.h>

enum {
	BOARD_TYPE_PCB106 = 0xAABBCD00,
	BOARD_TYPE_PCB105,
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

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, 0, 31, 0x10);
	phy_write(phydev, 0, 18, 0x80F0);
	while (phy_read(phydev, 0, 18) & 0x8000)
		;
	phy_write(phydev, 0, 14, 0x800);
	phy_write(phydev, 0, 31, 0);
	return 0;
}

static void do_board_detect(void)
{
	u16 gpio_in_reg;

	/* Set MDIO and MDC */
	mscc_gpio_set_alternate(9, 2);
	mscc_gpio_set_alternate(10, 2);

	/* Set GPIO page */
	mscc_phy_wr(1, 16, 31, 0x10);
	if (!mscc_phy_rd(1, 16, 15, &gpio_in_reg)) {
		if (gpio_in_reg & 0x200)
			gd->board_type = BOARD_TYPE_PCB106;
		else
			gd->board_type = BOARD_TYPE_PCB105;
	} else {
		gd->board_type = BOARD_TYPE_PCB105;
	}
	mscc_phy_wr(1, 16, 31, 0x0);
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB106 &&
	    strcmp(name, "serval_pcb106") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB105 &&
	    strcmp(name, "serval_pcb105") == 0)
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

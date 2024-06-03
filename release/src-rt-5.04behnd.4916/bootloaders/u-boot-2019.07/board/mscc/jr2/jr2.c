// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <led.h>
#include <miiphy.h>

enum {
	BOARD_TYPE_PCB110 = 0xAABBCE00,
	BOARD_TYPE_PCB111,
	BOARD_TYPE_PCB112,
};

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);
	clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
			ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
			ICPU_GENERAL_CTRL_IF_SI_OWNER(2));

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE;

	/* LED setup */
	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

static void vcoreiii_gpio_set_alternate(int gpio, int mode)
{
	u32 mask;
	u32 val0, val1;
	void __iomem *reg0, *reg1;

	if (gpio < 32) {
		mask = BIT(gpio);
		reg0 = BASE_DEVCPU_GCB + GPIO_GPIO_ALT(0);
		reg1 = BASE_DEVCPU_GCB + GPIO_GPIO_ALT(1);
	} else {
		gpio -= 32;
		mask = BIT(gpio);
		reg0 = BASE_DEVCPU_GCB + GPIO_GPIO_ALT1(0);
		reg1 = BASE_DEVCPU_GCB + GPIO_GPIO_ALT1(1);
	}
	val0 = readl(reg0);
	val1 = readl(reg1);
	if (mode == 1) {
		writel(val0 | mask, reg0);
		writel(val1 & ~mask, reg1);
	} else if (mode == 2) {
		writel(val0 & ~mask, reg0);
		writel(val1 | mask, reg1);
	} else if (mode == 3) {
		writel(val0 | mask, reg0);
		writel(val1 | mask, reg1);
	} else {
		writel(val0 & ~mask, reg0);
		writel(val1 & ~mask, reg1);
	}
}

int board_phy_config(struct phy_device *phydev)
{
	if (gd->board_type == BOARD_TYPE_PCB110 ||
	    gd->board_type == BOARD_TYPE_PCB112) {
		phy_write(phydev, 0, 31, 0x10);
		phy_write(phydev, 0, 18, 0x80F0);
		while (phy_read(phydev, 0, 18) & 0x8000)
			;
		phy_write(phydev, 0, 31, 0);
	}
	if (gd->board_type == BOARD_TYPE_PCB111) {
		phy_write(phydev, 0, 31, 0x10);
		phy_write(phydev, 0, 18, 0x80A0);
		while (phy_read(phydev, 0, 18) & 0x8000)
			;
		phy_write(phydev, 0, 14, 0x800);
		phy_write(phydev, 0, 31, 0);
	}

	return 0;
}

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	vcoreiii_gpio_set_alternate(10, 1);
	vcoreiii_gpio_set_alternate(11, 1);
}

static void do_board_detect(void)
{
	int i;
	u16 pval;

	/* MIIM 1 + 2  MDC/MDIO */
	for (i = 56; i < 60; i++)
		vcoreiii_gpio_set_alternate(i, 1);

	/* small delay for settling the pins */
	mdelay(30);

	if (mscc_phy_rd(0, 0x10, 0x3, &pval) == 0 &&
	    ((pval >> 4) & 0x3F) == 0x3c) {
		gd->board_type = BOARD_TYPE_PCB112; /* Serval2-NID */
	} else if (mscc_phy_rd(1, 0x0, 0x3, &pval) == 0 &&
		   ((pval >> 4) & 0x3F) == 0x3c) {
		gd->board_type = BOARD_TYPE_PCB110; /* Jr2-24 */
	} else {
		/* Fall-back */
		gd->board_type = BOARD_TYPE_PCB111; /* Jr2-48 */
	}
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB110 &&
	    strcmp(name, "jr2_pcb110") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB111 &&
	    strcmp(name, "jr2_pcb111") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB112 &&
	    strcmp(name, "serval2_pcb112") == 0)
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

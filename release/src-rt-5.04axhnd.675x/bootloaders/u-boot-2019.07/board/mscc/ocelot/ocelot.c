// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <environment.h>
#include <spi.h>
#include <led.h>
#include <wait_bit.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_PCB120 = 0xAABBCC00,
	BOARD_TYPE_PCB123,
};

void mscc_switch_reset(bool enter)
{
	/* Nasty workaround to avoid GPIO19 (DDR!) being reset */
	mscc_gpio_set_alternate(19, 2);

	debug("applying SwC reset\n");

	writel(ICPU_RESET_CORE_RST_PROTECT, BASE_CFG + ICPU_RESET);
	writel(PERF_SOFT_RST_SOFT_CHIP_RST, BASE_DEVCPU_GCB + PERF_SOFT_RST);

	if (wait_for_bit_le32(BASE_DEVCPU_GCB + PERF_SOFT_RST,
			      PERF_SOFT_RST_SOFT_CHIP_RST, false, 5000, false))
		pr_err("Tiemout while waiting for switch reset\n");

	/*
	 * Reset GPIO19 mode back as regular GPIO, output, high (DDR
	 * not reset) (Order is important)
	 */
	setbits_le32(BASE_DEVCPU_GCB + PERF_GPIO_OE, BIT(19));
	writel(BIT(19), BASE_DEVCPU_GCB + PERF_GPIO_OUT_SET);
	mscc_gpio_set_alternate(19, 0);
}

int board_phy_config(struct phy_device *phydev)
{
	if (gd->board_type == BOARD_TYPE_PCB123)
		return 0;

	phy_write(phydev, 0, 31, 0x10);
	phy_write(phydev, 0, 18, 0x80F0);
	while (phy_read(phydev, 0, 18) & 0x8000)
		;
	phy_write(phydev, 0, 31, 0);

	return 0;
}

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	mscc_gpio_set_alternate(6, 1);
	mscc_gpio_set_alternate(7, 1);
}

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

static void do_board_detect(void)
{
	u16 dummy = 0;

	/* Enable MIIM */
	mscc_gpio_set_alternate(14, 1);
	mscc_gpio_set_alternate(15, 1);
	if (mscc_phy_rd(1, 0, 0, &dummy) == 0)
		gd->board_type = BOARD_TYPE_PCB120;
	else
		gd->board_type = BOARD_TYPE_PCB123;
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_PCB120 &&
	    strcmp(name, "ocelot_pcb120") == 0)
		return 0;

	if (gd->board_type == BOARD_TYPE_PCB123 &&
	    strcmp(name, "ocelot_pcb123") == 0)
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

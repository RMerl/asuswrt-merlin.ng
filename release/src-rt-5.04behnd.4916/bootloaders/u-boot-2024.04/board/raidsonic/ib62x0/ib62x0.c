// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka@openwrt.org>
 * Simon Baatz <gmbnomis@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include "ib62x0.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(IB62x0_OE_VAL_LOW,
			  IB62x0_OE_VAL_HIGH,
			  IB62x0_OE_LOW, IB62x0_OE_HIGH);

	/* Set SATA activity LEDs to default off */
	writel(MVSATAHC_LED_POLARITY_CTRL, MVSATAHC_LED_CONF_REG);
	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP22_GPIO,     /* Power LED red */
		MPP24_GPIO,     /* Power off device */
		MPP25_GPIO,     /* Power LED green */
		MPP27_GPIO,     /* USB transfer LED */
		MPP28_GPIO,     /* Reset button */
		MPP29_GPIO,     /* USB Copy button */
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021-2022 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2013-2021 Suriyan Ramasami <suriyan.r@gmail.com>
 *
 * Based on dockstar.c originally written by
 * Copyright (C) 2010  Eric C. Cooper <ecc@cmu.edu>
 *
 * Based on sheevaplug.c originally written by
 * Prafulla Wadaskar <prafulla@marvell.com>
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <bootstage.h>
#include <init.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_UART0_RTS,
		MPP9_UART0_CTS,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_SD_CLK,
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GPIO,
		MPP21_GPIO,
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_TSMP9,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
		0
	};

	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(GOFLEXHOME_OE_VAL_LOW,
			  GOFLEXHOME_OE_VAL_HIGH,
			  GOFLEXHOME_OE_LOW, GOFLEXHOME_OE_HIGH);
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return cpu_eth_init(bis);
}

int board_init(void)
{
	/*
	 * arch number of board
	 */
	gd->bd->bi_arch_number = MACH_TYPE_GOFLEXHOME;

	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#if CONFIG_IS_ENABLED(BOOTSTAGE)
#define GREEN_LED	BIT(14)
#define ORANGE_LED	BIT(15)
#define BOTH_LEDS	(GREEN_LED | ORANGE_LED)
#define NEITHER_LED	0

static void set_leds(u32 leds, u32 blinking)
{
	struct kwgpio_registers *r;
	u32 oe;
	u32 bl;

	r = (struct kwgpio_registers *)MVEBU_GPIO1_BASE;
	oe = readl(&r->oe) | BOTH_LEDS;
	writel(oe & ~leds, &r->oe);	/* active low */
	bl = readl(&r->blink_en) & ~BOTH_LEDS;
	writel(bl | blinking, &r->blink_en);
}

void show_boot_progress(int val)
{
	switch (val) {
	case BOOTSTAGE_ID_RUN_OS:		/* booting Linux */
		set_leds(BOTH_LEDS, NEITHER_LED);
		break;
	case BOOTSTAGE_ID_NET_ETH_START:	/* Ethernet initialization */
		set_leds(GREEN_LED, GREEN_LED);
		break;
	default:
		if (val < 0)	/* error */
			set_leds(ORANGE_LED, ORANGE_LED);
		break;
	}
}
#endif

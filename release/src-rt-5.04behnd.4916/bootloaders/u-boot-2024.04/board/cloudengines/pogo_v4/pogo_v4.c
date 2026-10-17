// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2022 Tony Dinh <mibodhi@gmail.com>
 *
 * Based on
 * Copyright (C) 2012 David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <bootstage.h>
#include <command.h>
#include <init.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/* GPIO configuration */
#define POGO_V4_OE_LOW				(~(0))
#define POGO_V4_OE_HIGH				(~(0))
#define POGO_V4_OE_VAL_LOW			BIT(29)
#define POGO_V4_OE_VAL_HIGH			0

/* button */
#define BTN_EJECT				29

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(POGO_V4_OE_VAL_LOW,
			  POGO_V4_OE_VAL_HIGH,
			  POGO_V4_OE_LOW, POGO_V4_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
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
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP22_GPIO,	/* Green LED */
		MPP23_GPIO,
		MPP24_GPIO,	/* Red LED */
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_GPIO,	/* Eject button */
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,	/* FR6192 has only 36 GPIOs */
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return cpu_eth_init(bis);
}

int board_late_init(void)
{
	/* Do late init to ensure successful enumeration of XHCI devices */
	pci_init();
	return 0;
}

int board_init(void)
{
	/* Boot parameters address */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#if CONFIG_IS_ENABLED(BOOTSTAGE)
#define GREEN_LED	BIT(22)
#define RED_LED		BIT(24)
#define BOTH_LEDS	(GREEN_LED | RED_LED)
#define NEITHER_LED	0

static void set_leds(u32 leds, u32 blinking)
{
	struct kwgpio_registers *r;
	u32 oe;
	u32 bl;

	r = (struct kwgpio_registers *)MVEBU_GPIO0_BASE;
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
			set_leds(RED_LED, RED_LED);
		break;
	}
}
#endif

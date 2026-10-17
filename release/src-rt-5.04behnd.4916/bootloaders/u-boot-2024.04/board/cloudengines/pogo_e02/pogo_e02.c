// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2012
 * David Purdy <david.c.purdy@gmail.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/global_data.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/* GPIO configuration */
#define POGO_E02_OE_LOW			(~(0))
#define POGO_E02_OE_HIGH		(~(0))
#define POGO_E02_OE_VAL_LOW		BIT(29)
#define POGO_E02_OE_VAL_HIGH		0

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(POGO_E02_OE_VAL_LOW,
			  POGO_E02_OE_VAL_HIGH,
			  POGO_E02_OE_LOW, POGO_E02_OE_HIGH);

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
		MPP29_TSMP9,	/* USB Power Enable */
		MPP48_GPIO,	/* LED green */
		MPP49_GPIO,	/* LED orange */
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return cpu_eth_init(bis);
}

int board_init(void)
{
	/* Boot parameters address */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

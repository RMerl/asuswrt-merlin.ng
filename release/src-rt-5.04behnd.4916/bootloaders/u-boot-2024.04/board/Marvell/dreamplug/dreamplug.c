// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021-2022 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2011 Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 */

#include <common.h>
#include <init.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define DREAMPLUG_OE_LOW	(~(0))
#define DREAMPLUG_OE_HIGH	(~(0))
#define DREAMPLUG_OE_VAL_LOW	0
#define DREAMPLUG_OE_VAL_HIGH	(0xf << 16) /* 4 LED Pins high */

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(DREAMPLUG_OE_VAL_LOW,
			  DREAMPLUG_OE_VAL_HIGH,
			  DREAMPLUG_OE_LOW, DREAMPLUG_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,		/* SPI Flash */
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,	/* Serial */
		MPP11_UART0_RXD,
		MPP12_SD_CLK,		/* SDIO Slot */
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GE1_0,		/* Gigabit Ethernet */
		MPP21_GE1_1,
		MPP22_GE1_2,
		MPP23_GE1_3,
		MPP24_GE1_4,
		MPP25_GE1_5,
		MPP26_GE1_6,
		MPP27_GE1_7,
		MPP28_GE1_8,
		MPP29_GE1_9,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GE1_14,
		MPP35_GE1_15,
		MPP36_GPIO,		/* 7 external GPIO pins (36 - 45) */
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_TDM_SPI_SCK,
		MPP41_TDM_SPI_MISO,
		MPP42_TDM_SPI_MOSI,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,		/* Bluetooth LED */
		MPP48_GPIO,		/* Wifi LED */
		MPP49_GPIO,		/* Wifi AP LED */
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
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

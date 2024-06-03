// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009-2012
 * Wojciech Dubowik <wojciech.dubowik@neratec.com>
 * Luka Perkov <luka@openwrt.org>
 */

#include <common.h>
#include <miiphy.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include "iconnect.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(ICONNECT_OE_VAL_LOW,
			  ICONNECT_OE_VAL_HIGH,
			  ICONNECT_OE_LOW, ICONNECT_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,	/* Reset signal */
		MPP7_GPO,
		MPP8_TW_SDA,		/* I2C */
		MPP9_TW_SCK,		/* I2C */
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,		/* Reset button */
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GE1_0,
		MPP21_GE1_1,
		MPP22_GE1_2,
		MPP23_GE1_3,
		MPP24_GE1_4,
		MPP25_GE1_5,
		MPP26_GE1_6,
		MPP27_GE1_7,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GE1_14,
		MPP35_GPIO,		/* OTB button */
		MPP36_AUDIO_SPDIFI,
		MPP37_AUDIO_SPDIFO,
		MPP38_GPIO,
		MPP39_TDM_SPI_CS0,
		MPP40_TDM_SPI_SCK,
		MPP41_GPIO,		/* LED brightness */
		MPP42_GPIO,		/* LED power (blue) */
		MPP43_GPIO,		/* LED power (red) */
		MPP44_GPIO,		/* LED USB 1 */
		MPP45_GPIO,		/* LED USB 2 */
		MPP46_GPIO,		/* LED USB 3 */
		MPP47_GPIO,		/* LED USB 4 */
		MPP48_GPIO,		/* LED OTB */
		MPP49_GPIO,
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

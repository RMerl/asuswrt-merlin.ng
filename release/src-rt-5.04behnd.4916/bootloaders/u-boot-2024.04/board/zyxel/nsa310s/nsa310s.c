// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, 2021-2022 Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2015 Gerald Kerma <dreagle@doukki.net>
 */

#include <common.h>
#include <init.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * low GPIO's
 */
#define HDD1_GREEN_LED		BIT(16)
#define HDD1_RED_LED		BIT(13)
#define USB_GREEN_LED		BIT(15)
#define USB_POWER		BIT(21)
#define SYS_GREEN_LED		BIT(28)
#define SYS_ORANGE_LED		BIT(29)

#define COPY_GREEN_LED		BIT(22)
#define COPY_RED_LED		BIT(23)

#define PIN_USB_GREEN_LED	15
#define PIN_USB_POWER		21

#define NSA310S_OE_LOW		(~(0))
#define NSA310S_VAL_LOW		(SYS_GREEN_LED | USB_POWER)

/*
 * high GPIO's
 */
#define HDD2_GREEN_LED		BIT(2)
#define HDD2_POWER		BIT(1)

#define NSA310S_OE_HIGH		(~(0))
#define NSA310S_VAL_HIGH	(HDD2_POWER)

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(NSA310S_VAL_LOW, NSA310S_VAL_HIGH,
			  NSA310S_OE_LOW, NSA310S_OE_HIGH);

	/* (all LEDs & power off active high) */
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
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_GPIO,
		MPP14_GPIO,
		MPP15_GPIO,
		MPP16_GPIO,
		MPP17_GPIO,
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
		MPP29_GPIO,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return cpu_eth_init(bis);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2023 Tony Dinh <mibodhi@gmail.com>
 *
 * Based on
 * Copyright (C) 2014  Jason Plum <jplum@archlinuxarm.org>
 *
 * Based on nsa320.c originall written by
 * Copyright (C) 2012  Peter Schildmann <linux@schildmann.info>
 *
 * Based on guruplug.c originally written by
 * Siddarth Gore <gores@marvell.com>
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include <bootstage.h>
#include <command.h>
#include <init.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/* low GPIO's */
#define HDD2_GREEN_LED		BIT(12)
#define HDD2_RED_LED		BIT(13)
#define USB_GREEN_LED		BIT(15)
#define USB_POWER		BIT(21)
#define SYS_GREEN_LED		BIT(28)
#define SYS_ORANGE_LED		BIT(29)

#define PIN_USB_GREEN_LED	15
#define PIN_USB_POWER		21

#define NSA325_OE_LOW		(~(HDD2_GREEN_LED | HDD2_RED_LED |	\
				   USB_GREEN_LED | USB_POWER |		\
				   SYS_GREEN_LED | SYS_ORANGE_LED))
#define NSA325_VAL_LOW		(SYS_GREEN_LED | USB_POWER)

/* high GPIO's */
#define COPY_GREEN_LED		BIT(7)
#define COPY_RED_LED		BIT(8)
#define HDD1_GREEN_LED		BIT(9)
#define HDD1_RED_LED		BIT(10)
#define HDD2_POWER		BIT(15)
#define WATCHDOG_SIGNAL		BIT(14)

#define NSA325_OE_HIGH		(~(COPY_GREEN_LED | COPY_RED_LED | \
				   HDD1_GREEN_LED | HDD1_RED_LED | HDD2_POWER | WATCHDOG_SIGNAL))
#define NSA325_VAL_HIGH		(WATCHDOG_SIGNAL | HDD2_POWER)

#define BTN_POWER				46
#define BTN_RESET				36
#define BTN_COPY				37

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(NSA325_VAL_LOW, NSA325_VAL_HIGH,
			  NSA325_OE_LOW, NSA325_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	/* (all LEDs & power off active high) */
	u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,		/* PCF8563 RTC chip   */
		MPP9_TW_SCK,		/* connected to TWSI  */
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,		/* HDD2 LED (green)   */
		MPP13_GPIO,		/* HDD2 LED (red)     */
		MPP14_GPIO,		/* MCU DATA pin (in)  */
		MPP15_GPIO,		/* USB LED (green)    */
		MPP16_GPIO,		/* MCU CLK pin (out)  */
		MPP17_GPIO,		/* MCU ACT pin (out)  */
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GPIO,
		MPP21_GPIO,		/* USB power          */
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,		/* SYS LED (green)    */
		MPP29_GPIO,		/* SYS LED (orange)   */
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,		/* reset button       */
		MPP37_GPIO,		/* copy button        */
		MPP38_GPIO,		/* VID B0             */
		MPP39_GPIO,		/* COPY LED (green)   */
		MPP40_GPIO,		/* COPY LED (red)     */
		MPP41_GPIO,		/* HDD1 LED (green)   */
		MPP42_GPIO,		/* HDD1 LED (red)     */
		MPP43_GPIO,		/* HTP pin            */
		MPP44_GPIO,		/* buzzer             */
		MPP45_GPIO,		/* VID B1             */
		MPP46_GPIO,		/* power button       */
		MPP47_GPIO,		/* HDD2 power         */
		MPP48_GPIO,		/* power off          */
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

int board_late_init(void)
{
	/* Do late init to ensure successful enumeration of XHCI devices */
	pci_init();
	return 0;
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int val)
{
	struct kwgpio_registers *gpio0 = (struct kwgpio_registers *)MVEBU_GPIO0_BASE;
	u32 dout0 = readl(&gpio0->dout);
	u32 blen0 = readl(&gpio0->blink_en);

	struct kwgpio_registers *gpio1 = (struct kwgpio_registers *)MVEBU_GPIO1_BASE;
	u32 dout1 = readl(&gpio1->dout);
	u32 blen1 = readl(&gpio1->blink_en);

	switch (val) {
	case BOOTSTAGE_ID_DECOMP_IMAGE:
		writel(blen0 & ~(SYS_GREEN_LED | SYS_ORANGE_LED), &gpio0->blink_en);
		writel((dout0 & ~SYS_GREEN_LED) | SYS_ORANGE_LED, &gpio0->dout);
		break;
	case BOOTSTAGE_ID_RUN_OS:
		writel(dout0 & ~SYS_ORANGE_LED, &gpio0->dout);
		writel(blen0 | SYS_GREEN_LED, &gpio0->blink_en);
		break;
	case BOOTSTAGE_ID_NET_START:
		writel(dout1 & ~COPY_RED_LED, &gpio1->dout);
		writel((blen1 & ~COPY_RED_LED) | COPY_GREEN_LED, &gpio1->blink_en);
		break;
	case BOOTSTAGE_ID_NET_LOADED:
		writel(blen1 & ~(COPY_RED_LED | COPY_GREEN_LED), &gpio1->blink_en);
		writel((dout1 & ~COPY_RED_LED) | COPY_GREEN_LED, &gpio1->dout);
		break;
	case -BOOTSTAGE_ID_NET_NETLOOP_OK:
	case -BOOTSTAGE_ID_NET_LOADED:
		writel(dout1 & ~COPY_GREEN_LED, &gpio1->dout);
		writel((blen1 & ~COPY_GREEN_LED) | COPY_RED_LED, &gpio1->blink_en);
		break;
	default:
		if (val < 0) {
			/* error */
			printf("Error occurred, error code = %d\n", -val);
			writel(dout0 & ~SYS_GREEN_LED, &gpio0->dout);
			writel(blen0 | SYS_ORANGE_LED, &gpio0->blink_en);
		}
		break;
	}
}
#endif /* CONFIG_SHOW_BOOT_PROGRESS */

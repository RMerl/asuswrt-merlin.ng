// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>

#include "net2big_v2.h"
#include "../common/common.h"
#include "../common/cpld-gpio-bus.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* GPIO configuration */
	mvebu_config_gpio(NET2BIG_V2_OE_VAL_LOW, NET2BIG_V2_OE_VAL_HIGH,
			  NET2BIG_V2_OE_LOW, NET2BIG_V2_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,		/* Request power-off */
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP13_GPIO,		/* Rear power switch (on|auto) */
		MPP14_GPIO,		/* USB fuse alarm */
		MPP15_GPIO,		/* Rear power switch (auto|off) */
		MPP16_GPIO,		/* SATA HDD1 power */
		MPP17_GPIO,		/* SATA HDD2 power */
		MPP20_SATA1_ACTn,
		MPP21_SATA0_ACTn,
		MPP24_GPIO,		/* USB mode select */
		MPP26_GPIO,		/* USB device vbus */
		MPP28_GPIO,		/* USB enable host vbus */
		MPP29_GPIO,		/* CPLD GPIO bus ALE */
		MPP34_GPIO,		/* Rear Push button 0=on 1=off */
		MPP35_GPIO,		/* Inhibit switch power-off */
		MPP36_GPIO,		/* SATA HDD1 presence */
		MPP37_GPIO,		/* SATA HDD2 presence */
		MPP40_GPIO,		/* eSATA presence */
		MPP44_GPIO,		/* CPLD GPIO bus (data 0) */
		MPP45_GPIO,		/* CPLD GPIO bus (data 1) */
		MPP46_GPIO,		/* CPLD GPIO bus (data 2) */
		MPP47_GPIO,		/* CPLD GPIO bus (addr 0) */
		MPP48_GPIO,		/* CPLD GPIO bus (addr 1) */
		MPP49_GPIO,		/* CPLD GPIO bus (addr 2) */
		0
	};

	kirkwood_mpp_conf(kwmpp_config, NULL);

	return 0;
}

int board_init(void)
{
	/* Machine number */
	gd->bd->bi_arch_number = MACH_TYPE_NET2BIG_V2;

	/* Boot parameters address */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#if defined(CONFIG_MISC_INIT_R)

#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_G762_ADDR)
/*
 * Start I2C fan (GMT G762 controller)
 */
static void init_fan(void)
{
	u8 data;

	i2c_set_bus_num(0);

	/* Enable open-loop and PWM modes */
	data = 0x20;
	if (i2c_write(CONFIG_SYS_I2C_G762_ADDR,
		      G762_REG_FAN_CMD1, 1, &data, 1) != 0)
		goto err;
	data = 0;
	if (i2c_write(CONFIG_SYS_I2C_G762_ADDR,
		      G762_REG_SET_CNT, 1, &data, 1) != 0)
		goto err;
	/*
	 * RPM to PWM (set_out register) fan speed conversion array:
	 * 0    0x00
	 * 1500	0x04
	 * 2800	0x08
	 * 3400	0x0C
	 * 3700	0x10
	 * 4400	0x20
	 * 4700	0x30
	 * 4800	0x50
	 * 5200	0x80
	 * 5400	0xC0
	 * 5500	0xFF
	 *
	 * Start fan at low speed (2800 RPM):
	 */
	data = 0x08;
	if (i2c_write(CONFIG_SYS_I2C_G762_ADDR,
		      G762_REG_SET_OUT, 1, &data, 1) != 0)
		goto err;

	return;
err:
	printf("Error: failed to start I2C fan @%02x\n",
	       CONFIG_SYS_I2C_G762_ADDR);
}
#else
static void init_fan(void) {}
#endif /* CONFIG_CMD_I2C && CONFIG_SYS_I2C_G762_ADDR */

#if defined(CONFIG_NET2BIG_V2) && defined(CONFIG_KIRKWOOD_GPIO)
/*
 * CPLD GPIO bus:
 *
 * - address register : bit [0-2] -> GPIO [47-49]
 * - data register    : bit [0-2] -> GPIO [44-46]
 * - enable register  : GPIO 29
 */
static unsigned cpld_gpio_bus_addr[] = { 47, 48, 49 };
static unsigned cpld_gpio_bus_data[] = { 44, 45, 46 };

static struct cpld_gpio_bus cpld_gpio_bus = {
	.addr		= cpld_gpio_bus_addr,
	.num_addr	= ARRAY_SIZE(cpld_gpio_bus_addr),
	.data		= cpld_gpio_bus_data,
	.num_data	= ARRAY_SIZE(cpld_gpio_bus_data),
	.enable		= 29,
};

/*
 * LEDs configuration:
 *
 * The LEDs are controlled by a CPLD and can be configured through
 * the CPLD GPIO bus.
 *
 * Address register selection:
 *
 * addr | register
 * ----------------------------
 *   0  | front LED
 *   1  | front LED brightness
 *   2  | SATA LED brightness
 *   3  | SATA0 LED
 *   4  | SATA1 LED
 *   5  | SATA2 LED
 *   6  | SATA3 LED
 *   7  | SATA4 LED
 *
 * Data register configuration:
 *
 * data | LED brightness
 * -------------------------------------------------
 *   0  | min (off)
 *   -  | -
 *   7  | max
 *
 * data | front LED mode
 * -------------------------------------------------
 *   0  | fix off
 *   1  | fix blue on
 *   2  | fix red on
 *   3  | blink blue on=1 sec and blue off=1 sec
 *   4  | blink red on=1 sec and red off=1 sec
 *   5  | blink blue on=2.5 sec and red on=0.5 sec
 *   6  | blink blue on=1 sec and red on=1 sec
 *   7  | blink blue on=0.5 sec and blue off=2.5 sec
 *
 * data | SATA LED mode
 * -------------------------------------------------
 *   0  | fix off
 *   1  | SATA activity blink
 *   2  | fix red on
 *   3  | blink blue on=1 sec and blue off=1 sec
 *   4  | blink red on=1 sec and red off=1 sec
 *   5  | blink blue on=2.5 sec and red on=0.5 sec
 *   6  | blink blue on=1 sec and red on=1 sec
 *   7  | fix blue on
 */
static void init_leds(void)
{
	/* Enable the front blue LED */
	cpld_gpio_bus_write(&cpld_gpio_bus, 0, 1);
	cpld_gpio_bus_write(&cpld_gpio_bus, 1, 3);

	/* Configure SATA LEDs to blink in relation with the SATA activity */
	cpld_gpio_bus_write(&cpld_gpio_bus, 3, 1);
	cpld_gpio_bus_write(&cpld_gpio_bus, 4, 1);
	cpld_gpio_bus_write(&cpld_gpio_bus, 2, 3);
}
#else
static void init_leds(void) {}
#endif /* CONFIG_NET2BIG_V2 && CONFIG_KIRKWOOD_GPIO */

int misc_init_r(void)
{
	init_fan();
#if defined(CONFIG_CMD_I2C) && defined(CONFIG_SYS_I2C_EEPROM_ADDR)
	if (!env_get("ethaddr")) {
		uchar mac[6];
		if (lacie_read_mac_address(mac) == 0)
			eth_env_set_enetaddr("ethaddr", mac);
	}
#endif
	init_leds();

	return 0;
}
#endif /* CONFIG_MISC_INIT_R */

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
/* Configure and initialize PHY */
void reset_phy(void)
{
	mv_phy_88e1116_init("egiga0", 8);
}
#endif

#if defined(CONFIG_KIRKWOOD_GPIO)
/* Return GPIO push button status */
static int
do_read_push_button(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return !kw_gpio_get_value(NET2BIG_V2_GPIO_PUSH_BUTTON);
}

U_BOOT_CMD(button, 1, 1, do_read_push_button,
	   "Return GPIO push button status 0=off 1=on", "");
#endif

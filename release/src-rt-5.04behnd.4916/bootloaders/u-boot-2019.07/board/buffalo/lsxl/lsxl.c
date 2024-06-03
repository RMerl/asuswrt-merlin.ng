// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 *
 * Based on sheevaplug/sheevaplug.c by
 *   Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <environment.h>
#include <net.h>
#include <malloc.h>
#include <netdev.h>
#include <miiphy.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/soc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mpp.h>
#include <asm/arch/gpio.h>

#include "lsxl.h"

/*
 * Rescue mode
 *
 * Selected by holding the push button for 3 seconds, while powering on
 * the device.
 *
 * These linkstations don't have a (populated) serial port. There is no
 * way to access an (unmodified) board other than using the netconsole. If
 * you want to recover from a bad environment setting or an empty environment,
 * you can do this only with a working network connection. Therefore, a random
 * ethernet address is generated if none is set and a DHCP request is sent.
 * After a successful DHCP response is received, the network settings are
 * configured and the ncip is unset. Therefore, all netconsole packets are
 * broadcasted.
 * Additionally, the bootsource is set to 'rescue'.
 */

#ifndef CONFIG_ENV_OVERWRITE
# error "You need to set CONFIG_ENV_OVERWRITE"
#endif

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the below configuration configures mainly initial LED status
	 */
	mvebu_config_gpio(LSXL_OE_VAL_LOW,
			  LSXL_OE_VAL_HIGH,
			  LSXL_OE_LOW, LSXL_OE_HIGH);

	/*
	 * Multi-Purpose Pins Functionality configuration
	 * These strappings are taken from the original vendor uboot port.
	 */
	static const u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_UART0_RXD,
		MPP5_UART0_TXD,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_GPIO,
		MPP9_GPIO,
		MPP10_GPO,		/* HDD power */
		MPP11_GPIO,		/* USB Vbus enable */
		MPP12_SD_CLK,
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_GPO,		/* fan speed high */
		MPP19_GPO,		/* fan speed low */
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
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,		/* function LED */
		MPP37_GPIO,		/* alarm LED */
		MPP38_GPIO,		/* info LED */
		MPP39_GPIO,		/* power LED */
		MPP40_GPIO,		/* fan alarm */
		MPP41_GPIO,		/* funtion button */
		MPP42_GPIO,		/* power switch */
		MPP43_GPIO,		/* power auto switch */
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,		/* function red LED */
		MPP49_GPIO,
		0
	};

	kirkwood_mpp_conf(kwmpp_config, NULL);

	return 0;
}

#define LED_OFF             0
#define LED_ALARM_ON        1
#define LED_ALARM_BLINKING  2
#define LED_POWER_ON        3
#define LED_POWER_BLINKING  4
#define LED_INFO_ON         5
#define LED_INFO_BLINKING   6

static void __set_led(int blink_alarm, int blink_info, int blink_power,
		int value_alarm, int value_info, int value_power)
{
	kw_gpio_set_blink(GPIO_ALARM_LED, blink_alarm);
	kw_gpio_set_blink(GPIO_INFO_LED, blink_info);
	kw_gpio_set_blink(GPIO_POWER_LED, blink_power);
	kw_gpio_set_value(GPIO_ALARM_LED, value_alarm);
	kw_gpio_set_value(GPIO_INFO_LED, value_info);
	kw_gpio_set_value(GPIO_POWER_LED, value_power);
}

static void set_led(int state)
{
	switch (state) {
	case LED_OFF:
		__set_led(0, 0, 0, 1, 1, 1);
		break;
	case LED_ALARM_ON:
		__set_led(0, 0, 0, 0, 1, 1);
		break;
	case LED_ALARM_BLINKING:
		__set_led(1, 0, 0, 1, 1, 1);
		break;
	case LED_INFO_ON:
		__set_led(0, 0, 0, 1, 0, 1);
		break;
	case LED_INFO_BLINKING:
		__set_led(0, 1, 0, 1, 1, 1);
		break;
	case LED_POWER_ON:
		__set_led(0, 0, 0, 1, 1, 0);
		break;
	case LED_POWER_BLINKING:
		__set_led(0, 0, 1, 1, 1, 1);
		break;
	}
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	set_led(LED_POWER_BLINKING);

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
static void check_power_switch(void)
{
	if (kw_gpio_get_value(GPIO_POWER_SWITCH)) {
		/* turn off fan, HDD and USB power */
		kw_gpio_set_value(GPIO_HDD_POWER, 0);
		kw_gpio_set_value(GPIO_USB_VBUS, 0);
		kw_gpio_set_value(GPIO_FAN_HIGH, 1);
		kw_gpio_set_value(GPIO_FAN_LOW, 1);
		set_led(LED_OFF);

		/* loop until released */
		while (kw_gpio_get_value(GPIO_POWER_SWITCH))
			;

		/* turn power on again */
		kw_gpio_set_value(GPIO_HDD_POWER, 1);
		kw_gpio_set_value(GPIO_USB_VBUS, 1);
		kw_gpio_set_value(GPIO_FAN_HIGH, 0);
		kw_gpio_set_value(GPIO_FAN_LOW, 0);
		set_led(LED_POWER_BLINKING);
	}
}

void check_enetaddr(void)
{
	uchar enetaddr[6];

	if (!eth_env_get_enetaddr("ethaddr", enetaddr)) {
		/* signal unset/invalid ethaddr to user */
		set_led(LED_INFO_BLINKING);
	}
}

static void erase_environment(void)
{
	struct spi_flash *flash;

	printf("Erasing environment..\n");
	flash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (!flash) {
		printf("Erasing flash failed\n");
		return;
	}

	spi_flash_erase(flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE);
	spi_flash_free(flash);
	do_reset(NULL, 0, 0, NULL);
}

static void rescue_mode(void)
{
	printf("Entering rescue mode..\n");
	env_set("bootsource", "rescue");
}

static void check_push_button(void)
{
	int i = 0;

	while (!kw_gpio_get_value(GPIO_FUNC_BUTTON)) {
		udelay(100000);
		i++;

		if (i == 10)
			set_led(LED_INFO_ON);

		if (i >= 100) {
			set_led(LED_INFO_BLINKING);
			break;
		}
	}

	if (i >= 100)
		erase_environment();
	else if (i >= 10)
		rescue_mode();
}

int misc_init_r(void)
{
	check_power_switch();
	check_enetaddr();
	check_push_button();

	return 0;
}
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int progress)
{
	if (progress > 0)
		return;

	/* this is not an error, eg. bootp with autoload=no will trigger this */
	if (progress == -BOOTSTAGE_ID_NET_LOADED)
		return;

	set_led(LED_ALARM_BLINKING);
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 *
 * Based on sheevaplug/sheevaplug.c by
 *   Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <bootstage.h>
#include <button.h>
#include <command.h>
#include <env.h>
#include <init.h>
#include <led.h>
#include <power/regulator.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mpp.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/delay.h>

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

DECLARE_GLOBAL_DATA_PTR;

static bool force_rescue_mode;

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

enum {
	LSXL_LED_OFF,
	LSXL_LED_ALARM,
	LSXL_LED_POWER,
	LSXL_LED_INFO,
};

static void __set_led(int alarm, int info, int power)
{
	struct udevice *led;
	int ret;

	ret = led_get_by_label("lsxl:red:alarm", &led);
	if (!ret)
		led_set_state(led, alarm);
	ret = led_get_by_label("lsxl:amber:info", &led);
	if (!ret)
		led_set_state(led, info);
	ret = led_get_by_label("lsxl:blue:power", &led);
	if (!ret)
		led_set_state(led, power);
}

static void set_led(int state)
{
	switch (state) {
	case LSXL_LED_OFF:
		__set_led(0, 0, 0);
		break;
	case LSXL_LED_ALARM:
		__set_led(1, 0, 0);
		break;
	case LSXL_LED_INFO:
		__set_led(0, 1, 0);
		break;
	case LSXL_LED_POWER:
		__set_led(0, 0, 1);
		break;
	}
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	set_led(LSXL_LED_POWER);

	return 0;
}

static void check_power_switch(void)
{
	struct udevice *power_button, *hdd_power, *usb_power;
	int ret;

	ret = button_get_by_label("Power-on Switch", &power_button);
	if (ret)
		goto err;

	ret = regulator_get_by_platname("HDD Power", &hdd_power);
	if (ret)
		goto err;

	ret = regulator_get_by_platname("USB Power", &usb_power);
	if (ret)
		goto err;

	if (button_get_state(power_button) == BUTTON_OFF) {
		ret = regulator_set_enable(hdd_power, false);
		if (ret)
			goto err;
		ret = regulator_set_enable(usb_power, false);
		if (ret)
			goto err;
		/* TODO: fan off */
		set_led(LSXL_LED_OFF);

		/* loop until released */
		while (button_get_state(power_button) == BUTTON_OFF)
			;

		/* turn power on again */
		ret = regulator_set_enable(hdd_power, true);
		if (ret)
			goto err;
		ret = regulator_set_enable(usb_power, true);
		if (ret)
			goto err;
		/* TODO: fan on */
		set_led(LSXL_LED_POWER);
	};

	return;
err:
	printf("error in %s\n", __func__);
}

void check_enetaddr(void)
{
	uchar enetaddr[6];

	if (!eth_env_get_enetaddr("ethaddr", enetaddr)) {
		/* signal unset/invalid ethaddr to user */
		set_led(LSXL_LED_INFO);
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
	struct udevice *func_button;
	int i = 0;

	int ret;

	ret = button_get_by_label("Function Button", &func_button);
	if (ret)
		goto err;

	while (button_get_state(func_button) == BUTTON_ON) {
		udelay(100000);
		i++;

		if (i == 10)
			set_led(LSXL_LED_INFO);

		if (i >= 100) {
			set_led(LSXL_LED_ALARM);
			break;
		}
	}

	if (i >= 100)
		erase_environment();
	else if (i >= 10)
		force_rescue_mode = true;

	return;
err:
	printf("error in %s\n", __func__);
}

int board_early_init_r(void)
{
	check_push_button();

	return 0;
}

int misc_init_r(void)
{
	check_power_switch();
	check_enetaddr();
	if (force_rescue_mode)
		rescue_mode();

	return 0;
}

#if CONFIG_IS_ENABLED(BOOTSTAGE)
void show_boot_progress(int progress)
{
	if (progress > 0)
		return;

	/* this is not an error, eg. bootp with autoload=no will trigger this */
	if (progress == -BOOTSTAGE_ID_NET_LOADED)
		return;

	set_led(LSXL_LED_ALARM);
}
#endif

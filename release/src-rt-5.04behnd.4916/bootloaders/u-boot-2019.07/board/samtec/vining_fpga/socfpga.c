// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <environment.h>
#include <asm/arch/reset_manager.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */
int board_late_init(void)
{
	const unsigned int phy_nrst_gpio = 0;
	const unsigned int usb_nrst_gpio = 35;
	int ret;

	status_led_set(1, CONFIG_LED_STATUS_ON);
	status_led_set(2, CONFIG_LED_STATUS_ON);

	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	ret = gpio_request(phy_nrst_gpio, "phy_nrst_gpio");
	if (!ret)
		gpio_direction_output(phy_nrst_gpio, 1);
	else
		printf("Cannot remove PHY from reset!\n");

	ret = gpio_request(usb_nrst_gpio, "usb_nrst_gpio");
	if (!ret)
		gpio_direction_output(usb_nrst_gpio, 1);
	else
		printf("Cannot remove USB from reset!\n");

	mdelay(50);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
int misc_init_r(void)
{
	uchar data[128];
	char str[32];
	u32 serial;
	int ret;

	/* EEPROM is at address 0x50 (at bus CONFIG_SYS_EEPROM_BUS_NUM). */
	ret = eeprom_read(0x50, 0, data, sizeof(data));
	if (ret) {
		puts("Cannot read I2C EEPROM.\n");
		return 0;
	}

	/* Check EEPROM signature. */
	if (!(data[0] == 0xa5 && data[1] == 0x5a)) {
		puts("Invalid I2C EEPROM signature.\n");
		env_set("unit_serial", "invalid");
		env_set("unit_ident", "VINing-xxxx-STD");
		env_set("hostname", "vining-invalid");
		return 0;
	}

	/* If 'unit_serial' is already set, do nothing. */
	if (!env_get("unit_serial")) {
		/* This field is Big Endian ! */
		serial = (data[0x54] << 24) | (data[0x55] << 16) |
			 (data[0x56] << 8) | (data[0x57] << 0);
		memset(str, 0, sizeof(str));
		sprintf(str, "%07i", serial);
		env_set("unit_serial", str);
	}

	if (!env_get("unit_ident")) {
		memset(str, 0, sizeof(str));
		memcpy(str, &data[0x2e], 18);
		env_set("unit_ident", str);
	}

	/* Set ethernet address from EEPROM. */
	if (!env_get("ethaddr") && is_valid_ethaddr(&data[0x62]))
		eth_env_set_enetaddr("ethaddr", &data[0x62]);

	return 0;
}
#endif

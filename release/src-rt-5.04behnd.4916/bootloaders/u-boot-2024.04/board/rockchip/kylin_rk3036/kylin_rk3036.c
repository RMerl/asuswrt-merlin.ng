// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch-rockchip/uart.h>
#include <asm/arch-rockchip/sdram_rk3036.h>
#include <asm/gpio.h>

void get_ddr_config(struct rk3036_ddr_config *config)
{
	/* K4B4G1646Q config */
	config->ddr_type = 3;
	config->rank = 1;
	config->cs0_row = 15;
	config->cs1_row = 15;

	/* 8bank */
	config->bank = 3;
	config->col = 10;

	/* 16bit bw */
	config->bw = 1;
}

#define FASTBOOT_KEY_GPIO 93

int fastboot_key_pressed(void)
{
	gpio_request(FASTBOOT_KEY_GPIO, "fastboot_key");
	gpio_direction_input(FASTBOOT_KEY_GPIO);
	return !gpio_get_value(FASTBOOT_KEY_GPIO);
}

#define ROCKCHIP_BOOT_MODE_FASTBOOT	0x5242C309

int rk_board_late_init(void)
{
	if (fastboot_key_pressed()) {
		printf("enter fastboot!\n");
		env_set("preboot", "setenv preboot; fastboot usb0");
	}

	return 0;
}

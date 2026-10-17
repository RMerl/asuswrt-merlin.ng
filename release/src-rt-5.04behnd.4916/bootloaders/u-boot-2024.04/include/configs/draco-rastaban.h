/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/include/configs/am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_RASTABAN_H
#define __CONFIG_RASTABAN_H

#include "siemens-am33x-common.h"

#define DDR_PLL_FREQ	303

/* FWD Button = 27
 * SRV Button = 87 */
#define BOARD_DFU_BUTTON_GPIO	27
#define GPIO_LAN9303_NRST	88	/* GPIO2_24 = gpio88 */
/* In dfu mode keep led1 on */
#define CFG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	"button_dfu0=27\0" \
	"button_dfu1=87\0" \
	"led0=3,0,1\0" \
	"led1=4,0,0\0" \
	"led2=5,0,1\0" \
	"led3=62,0,1\0" \
	"led4=60,0,1\0" \
	"led5=63,0,1\0"

 /* Physical Memory Map */
#define CFG_MAX_RAM_BANK_SIZE	(1024 << 20)	/* 1GB */

/* Default env settings */
#define CFG_EXTRA_ENV_SETTINGS \
	"hostname=rastaban\0" \
	"ubi_off=2048\0"\
	"nand_img_size=0x400000\0" \
	"optargs=\0" \
	"preboot=draco_led 0\0" \
	CFG_ENV_SETTINGS_BUTTONS_AND_LEDS \
	CFG_ENV_SETTINGS_V2 \
	CFG_ENV_SETTINGS_NAND_V2

#endif	/* ! __CONFIG_RASTABAN_H */

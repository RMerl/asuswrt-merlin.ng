/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 */

enum axp152_reg {
	AXP152_CHIP_VERSION = 0x3,
	AXP152_DCDC2_VOLTAGE = 0x23,
	AXP152_DCDC3_VOLTAGE = 0x27,
	AXP152_DCDC4_VOLTAGE = 0x2B,
	AXP152_LDO2_VOLTAGE = 0x2A,
	AXP152_SHUTDOWN = 0x32,
};

#define AXP152_POWEROFF			(1 << 7)

/* For axp_gpio.c */
#define AXP_GPIO0_CTRL			0x90
#define AXP_GPIO1_CTRL			0x91
#define AXP_GPIO2_CTRL			0x92
#define AXP_GPIO3_CTRL			0x93
#define AXP_GPIO_CTRL_OUTPUT_LOW		0x00 /* Drive pin low */
#define AXP_GPIO_CTRL_OUTPUT_HIGH		0x01 /* Drive pin high */
#define AXP_GPIO_CTRL_INPUT			0x02 /* Input */
#define AXP_GPIO_STATE			0x97
#define AXP_GPIO_STATE_OFFSET			0

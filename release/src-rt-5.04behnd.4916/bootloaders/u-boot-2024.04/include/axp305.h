/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 */

enum axp305_reg {
	AXP305_CHIP_VERSION = 0x3,
	AXP305_OUTPUT_CTRL1 = 0x10,
	AXP305_DCDCD_VOLTAGE = 0x15,
	AXP305_SHUTDOWN = 0x32,
};

#define AXP305_CHIP_VERSION_MASK	0xcf

#define AXP305_OUTPUT_CTRL1_DCDCD_EN	(1 << 3)

#define AXP305_POWEROFF			(1 << 7)

#define AXP_POWER_STATUS		0x00
#define AXP_POWER_STATUS_ALDO_IN		BIT(0)

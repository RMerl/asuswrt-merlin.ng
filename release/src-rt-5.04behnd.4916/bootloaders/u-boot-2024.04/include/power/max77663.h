/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef _MAX77663_H_
#define _MAX77663_H_

#define MAX77663_LDO_NUM		9
#define MAX77663_SD_NUM			5

/* Drivers name */
#define MAX77663_LDO_DRIVER		"max77663_ldo"
#define MAX77663_SD_DRIVER		"max77663_sd"
#define MAX77663_RST_DRIVER		"max77663_rst"
#define MAX77663_GPIO_DRIVER		"max77663_gpio"

/* Step-Down (SD) Regulator calculations */
#define SD_STATUS_MASK			0x30

#define SD0_VOLT_MAX_HEX		0x40
#define SD1_VOLT_MAX_HEX		0x4c
#define SD_VOLT_MAX_HEX			0xff
#define SD_VOLT_MIN_HEX			0x02

#define SD0_VOLT_MAX			1400000
#define SD1_VOLT_MAX			1550000
#define SD_VOLT_MAX			3787500
#define SD_VOLT_MIN			625000

#define SD_VOLT_BASE			600000

/* Low-Dropout Linear (LDO) Regulator calculations */
#define LDO_STATUS_MASK			0xc0
#define LDO_VOLT_MASK			0x3f
#define LDO_VOLT_MAX_HEX		0x3f

#define LDO01_VOLT_MAX			2375000
#define LDO4_VOLT_MAX			1587500
#define LDO_VOLT_MAX			3950000

#define LDO_VOLT_BASE			800000

#define MAX77663_REG_ONOFF_CFG1		0x41
#define   ONOFF_SFT_RST			BIT(7)
#define   ONOFF_PWR_OFF			BIT(1)

#endif /* _MAX77663_H_ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef _TPS80031_H_
#define _TPS80031_H_

#define TPS80031_LDO_NUM		9
#define TPS80031_SMPS_NUM		5

/* Drivers name */
#define TPS80031_LDO_DRIVER		"tps80031_ldo"
#define TPS80031_SMPS_DRIVER		"tps80031_smps"
#define TPS80031_RST_DRIVER		"tps80031_rst"

#define TPS80031_SMPS_OFFSET		0xe0
#define TPS80031_OFFSET_FLAG		BIT(0)

#define REGULATOR_STATUS_MASK		0x3
#define REGULATOR_MODE_ON		0x1

/* Switched-Mode Power Supply Regulator calculations */
#define SMPS_VOLT_MASK			0x3f
#define SMPS_VOLT_LINEAR_HEX		0x39
#define SMPS_VOLT_NLINEAR_HEX		0x3a
#define SMPS_VOLT_LINEAR		1300000
#define SMPS_VOLT_BASE			600000
#define SMPS_VOLT_BASE_OFFSET		700000

/* Low-Dropout Linear (LDO) Regulator calculations */
#define LDO_VOLT_MASK			0x3f
#define LDO_VOLT_MAX_HEX		0x18
#define LDO_VOLT_MIN_HEX		0x01
#define LDO_VOLT_MAX			3360000
#define LDO_VOLT_MIN			1018000
#define LDO_VOLT_BASE			916000

#define TPS80031_PHOENIX_DEV_ON		0x25
#define   SW_RESET			BIT(6)
#define   DEVOFF			BIT(0)

/* register groups */
enum {
	CTRL,
	VOLT,
	OFFSET,
};

#endif /* _TPS80031_H_ */

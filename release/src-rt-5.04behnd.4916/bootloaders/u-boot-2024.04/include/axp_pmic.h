/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * X-Powers AX Power Management IC support header
 */
#ifndef _AXP_PMIC_H_
#define _AXP_PMIC_H_

#include <stdbool.h>

#include <axp152.h>
#include <axp209.h>
#include <axp221.h>
#include <axp305.h>
#include <axp809.h>
#include <axp818.h>

#define AXP_PMIC_MODE_REG		0x3e
#define AXP_PMIC_MODE_I2C		0x00
#define AXP_PMIC_MODE_P2WI		0x3e
#define AXP_PMIC_MODE_RSB		0x7c

#define AXP_PMIC_PRI_DEVICE_ADDR	0x3a3
#define AXP_PMIC_PRI_RUNTIME_ADDR	0x2d
#define AXP_PMIC_SEC_DEVICE_ADDR	0x745
#define AXP_PMIC_SEC_RUNTIME_ADDR	0x3a

enum {
	AXP152_ID,
	AXP202_ID,
	AXP209_ID,
	AXP221_ID,
	AXP223_ID,
	AXP313_ID,
	AXP803_ID,
	AXP806_ID,
	AXP809_ID,
	AXP813_ID,
};

int axp_set_dcdc1(unsigned int mvolt);
int axp_set_dcdc2(unsigned int mvolt);
int axp_set_dcdc3(unsigned int mvolt);
int axp_set_dcdc4(unsigned int mvolt);
int axp_set_dcdc5(unsigned int mvolt);
int axp_set_aldo1(unsigned int mvolt);
int axp_set_aldo2(unsigned int mvolt);
int axp_set_aldo3(unsigned int mvolt);
int axp_set_aldo4(unsigned int mvolt);
int axp_set_dldo(int dldo_num, unsigned int mvolt);
int axp_set_eldo(int eldo_num, unsigned int mvolt);
int axp_set_fldo(int fldo_num, unsigned int mvolt);
int axp_set_sw(bool on);
int axp_init(void);
int axp_get_sid(unsigned int *sid);

#endif

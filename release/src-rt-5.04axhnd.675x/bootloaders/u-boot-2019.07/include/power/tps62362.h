/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Texas Instruments Incorporated - http://www.ti.com
 * Author: Felipe Balbi <balbi@ti.com>
 */

#ifndef __POWER_TPS62362_H__
#define __POWER_TPS62362_H__

/* I2C chip address */
#define TPS62362_I2C_ADDR			0x60

/* Registers */
#define TPS62362_SET0				0x00
#define TPS62362_SET1				0x01
#define TPS62362_SET2				0x02
#define TPS62362_SET3				0x03
#define TPS62362_NUM_REGS			4

#define TPS62362_DCDC_VOLT_SEL_0950MV		0x12
#define TPS62362_DCDC_VOLT_SEL_1100MV		0x21
#define TPS62362_DCDC_VOLT_SEL_1200MV		0x2b
#define TPS62362_DCDC_VOLT_SEL_1260MV		0x31
#define TPS62362_DCDC_VOLT_SEL_1330MV		0x38

int tps62362_voltage_update(unsigned char reg, unsigned char volt_sel);
int power_tps62362_init(unsigned char bus);
#endif	/* __POWER_TPS62362_H__ */

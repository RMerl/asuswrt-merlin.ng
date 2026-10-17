/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 */

#ifndef __POWER_TPS65218_H__
#define __POWER_TPS65218_H__

#include <linux/bitops.h>

/* I2C chip address */
#define TPS65218_CHIP_PM			0x24

/* Registers */
enum {
	TPS65218_CHIPID				= 0x00,
	TPS65218_INT1,
	TPS65218_INT2,
	TPS65218_INT_MASK1,
	TPS65218_INT_MASK2,
	TPS65218_STATUS,
	TPS65218_CONTROL,
	TPS65218_FLAG,
	TPS65218_PASSWORD			= 0x10,
	TPS65218_ENABLE1,
	TPS65218_ENABLE2,
	TPS65218_CONFIG1,
	TPS65218_CONFIG2,
	TPS65218_CONFIG3,
	TPS65218_DCDC1,
	TPS65218_DCDC2,
	TPS65218_DCDC3,
	TPS65218_DCDC4,
	TPS65218_SLEW,
	TPS65218_LDO1,
	TPS65218_SEQ1				= 0x20,
	TPS65218_SEQ2,
	TPS65218_SEQ3,
	TPS65218_SEQ4,
	TPS65218_SEQ5,
	TPS65218_SEQ6,
	TPS65218_SEQ7,
	TPS65218_PMIC_NUM_OF_REGS,
};

#define TPS65218_PROT_LEVEL_NONE		0x00
#define TPS65218_PROT_LEVEL_1			0x01
#define TPS65218_PROT_LEVEL_2			0x02

#define TPS65218_PASSWORD_LOCK_FOR_WRITE	0x00
#define TPS65218_PASSWORD_UNLOCK		0x7D

#define TPS65218_DCDC_GO			0x80

#define TPS65218_MASK_ALL_BITS			0xFF

#define TPS65218_DCDC_VSEL_MASK			0x3F

#define TPS65218_DCDC_VOLT_SEL_0950MV		0x0a
#define TPS65218_DCDC_VOLT_SEL_1100MV		0x19
#define TPS65218_DCDC_VOLT_SEL_1200MV		0x23
#define TPS65218_DCDC_VOLT_SEL_1260MV		0x29
#define TPS65218_DCDC_VOLT_SEL_1330MV		0x30
#define TPS65218_DCDC3_VOLT_SEL_1350MV		0x12
#define TPS65218_DCDC3_VOLT_SEL_1200MV		0xC

#define TPS65218_CC_STAT	(BIT(0) | BIT(1))
#define TPS65218_STATE		(BIT(2) | BIT(3))
#define TPS65218_PB_STATE	BIT(4)
#define TPS65218_AC_STATE	BIT(5)
#define TPS65218_EE		BIT(6)
#define TPS65218_FSEAL		BIT(7)

int tps65218_reg_read(uchar dest_reg, uchar *dest_val);
int tps65218_reg_write(uchar prot_level, uchar dest_reg, uchar dest_val,
		       uchar mask);
int tps65218_voltage_update(uchar dc_cntrl_reg, uchar volt_sel);
int tps65218_toggle_fseal(void);
int tps65218_lock_fseal(void);
int power_tps65218_init(unsigned char bus);
#endif	/* __POWER_TPS65218_H__ */

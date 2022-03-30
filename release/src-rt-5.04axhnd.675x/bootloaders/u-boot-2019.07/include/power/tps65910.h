/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2013
 * Texas Instruments, <www.ti.com>
 *
 * For more details, please see the TRM at http://www.ti.com/product/tps65910
 */
#ifndef __POWER_TPS65910_H__
#define __POWER_TPS65910_H__

#define MPU     0
#define CORE    1

#define TPS65910_SR_I2C_ADDR				0x12
#define TPS65910_CTRL_I2C_ADDR				0x2D

/* PMIC Register offsets */
enum {
	TPS65910_VDD1_REG				= 0x21,
	TPS65910_VDD1_OP_REG				= 0x22,
	TPS65910_VDD2_REG				= 0x24,
	TPS65910_VDD2_OP_REG				= 0x25,
	TPS65910_DEVCTRL_REG				= 0x3F,
};

/* VDD2 & VDD1 control register (VDD2_REG & VDD1_REG) */
#define TPS65910_VGAIN_SEL_MASK				(0x3 << 6)
#define TPS65910_ILMAX_MASK				(0x1 << 5)
#define TPS65910_TSTEP_MASK				(0x7 << 2)
#define TPS65910_ST_MASK				(0x3)

#define TPS65910_REG_VGAIN_SEL_X1			(0x0 << 6)
#define TPS65910_REG_VGAIN_SEL_X1_0			(0x1 << 6)
#define TPS65910_REG_VGAIN_SEL_X3			(0x2 << 6)
#define TPS65910_REG_VGAIN_SEL_X4			(0x3 << 6)

#define TPS65910_REG_ILMAX_1_0_A			(0x0 << 5)
#define TPS65910_REG_ILMAX_1_5_A			(0x1 << 5)

#define TPS65910_REG_TSTEP_				(0x0 << 2)
#define TPS65910_REG_TSTEP_12_5				(0x1 << 2)
#define TPS65910_REG_TSTEP_9_4				(0x2 << 2)
#define TPS65910_REG_TSTEP_7_5				(0x3 << 2)
#define TPS65910_REG_TSTEP_6_25				(0x4 << 2)
#define TPS65910_REG_TSTEP_4_7				(0x5 << 2)
#define TPS65910_REG_TSTEP_3_12				(0x6 << 2)
#define TPS65910_REG_TSTEP_2_5				(0x7 << 2)

#define TPS65910_REG_ST_OFF				(0x0)
#define TPS65910_REG_ST_ON_HI_POW			(0x1)
#define TPS65910_REG_ST_OFF_1				(0x2)
#define TPS65910_REG_ST_ON_LOW_POW			(0x3)


/* VDD2 & VDD1 voltage selection register. (VDD2_OP_REG & VDD1_OP_REG) */
#define TPS65910_OP_REG_SEL				(0x7F)

#define TPS65910_OP_REG_CMD_MASK			(0x1 << 7)
#define TPS65910_OP_REG_CMD_OP				(0x0 << 7)
#define TPS65910_OP_REG_CMD_SR				(0x1 << 7)

#define TPS65910_OP_REG_SEL_MASK			(0x7F)
#define TPS65910_OP_REG_SEL_0_9_5			(0x1F)	/* 0.9500 V */
#define TPS65910_OP_REG_SEL_1_1_0			(0x2B)	/* 1.1000 V */
#define TPS65910_OP_REG_SEL_1_1_3			(0x2E)	/* 1.1375 V */
#define TPS65910_OP_REG_SEL_1_2_0			(0x33)	/* 1.2000 V */
#define TPS65910_OP_REG_SEL_1_2_6			(0x38)	/* 1.2625 V */
#define TPS65910_OP_REG_SEL_1_3_2_5			(0x3D)	/* 1.3250 V */

/* Device control register . (DEVCTRL_REG) */
#define TPS65910_DEVCTRL_REG_SR_CTL_I2C_MASK		(0x1 << 4)
#define TPS65910_DEVCTRL_REG_SR_CTL_I2C_SEL_SR_I2C	(0x0 << 4)
#define TPS65910_DEVCTRL_REG_SR_CTL_I2C_SEL_CTL_I2C	(0x1 << 4)

int power_tps65910_init(unsigned char bus);
int tps65910_set_i2c_control(void);
int tps65910_voltage_update(unsigned int module, unsigned char vddx_op_vol_sel);
#endif	/* __POWER_TPS65910_H__ */

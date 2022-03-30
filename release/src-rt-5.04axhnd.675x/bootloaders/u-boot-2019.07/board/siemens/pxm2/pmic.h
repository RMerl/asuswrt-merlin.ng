/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 */
#ifndef PMIC_H
#define PMIC_H

/*
 * The PMIC on this board is a TPS65910.
 */

#define PMIC_SR_I2C_ADDR		0x12
#define PMIC_CTRL_I2C_ADDR		0x2D
/* PMIC Register offsets */
#define PMIC_VDD1_REG			0x21
#define PMIC_VDD1_OP_REG		0x22
#define PMIC_VDD2_REG			0x24
#define PMIC_VDD2_OP_REG		0x25
#define PMIC_DEVCTRL_REG		0x3f

/* VDD2 & VDD1 control register (VDD2_REG & VDD1_REG) */
#define PMIC_VGAIN_SEL_MASK		(0x3 << 6)
#define PMIC_ILMAX_MASK			(0x1 << 5)
#define PMIC_TSTEP_MASK			(0x7 << 2)
#define PMIC_ST_MASK			(0x3)

#define PMIC_REG_VGAIN_SEL_X1		(0x0 << 6)
#define PMIC_REG_VGAIN_SEL_X1_0		(0x1 << 6)
#define PMIC_REG_VGAIN_SEL_X3		(0x2 << 6)
#define PMIC_REG_VGAIN_SEL_X4		(0x3 << 6)

#define PMIC_REG_ILMAX_1_0_A		(0x0 << 5)
#define PMIC_REG_ILMAX_1_5_A		(0x1 << 5)

#define PMIC_REG_TSTEP_			(0x0 << 2)
#define PMIC_REG_TSTEP_12_5		(0x1 << 2)
#define PMIC_REG_TSTEP_9_4		(0x2 << 2)
#define PMIC_REG_TSTEP_7_5		(0x3 << 2)
#define PMIC_REG_TSTEP_6_25		(0x4 << 2)
#define PMIC_REG_TSTEP_4_7		(0x5 << 2)
#define PMIC_REG_TSTEP_3_12		(0x6 << 2)
#define PMIC_REG_TSTEP_2_5		(0x7 << 2)

#define PMIC_REG_ST_OFF			(0x0)
#define PMIC_REG_ST_ON_HI_POW		(0x1)
#define PMIC_REG_ST_OFF_1		(0x2)
#define PMIC_REG_ST_ON_LOW_POW		(0x3)


/* VDD2 & VDD1 voltage selection register. (VDD2_OP_REG & VDD1_OP_REG) */
#define PMIC_OP_REG_SEL				(0x7F)

#define PMIC_OP_REG_CMD_MASK			(0x1 << 7)
#define PMIC_OP_REG_CMD_OP			(0x0 << 7)
#define PMIC_OP_REG_CMD_SR			(0x1 << 7)

#define PMIC_OP_REG_SEL_MASK			(0x7F)
#define PMIC_OP_REG_SEL_1_1_3			(0x2E)	/* 1.1375 V */
#define PMIC_OP_REG_SEL_1_2_6			(0x38)	/* 1.2625 V */

/* Device control register . (DEVCTRL_REG) */
#define PMIC_DEVCTRL_REG_SR_CTL_I2C_MASK	(0x1 << 4)
#define PMIC_DEVCTRL_REG_SR_CTL_I2C_SEL_SR_I2C	(0x0 << 4)
#define PMIC_DEVCTRL_REG_SR_CTL_I2C_SEL_CTL_I2C	(0x1 << 4)

#endif

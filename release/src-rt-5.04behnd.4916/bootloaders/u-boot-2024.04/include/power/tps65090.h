/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __TPS65090_PMIC_H_
#define __TPS65090_PMIC_H_

/* I2C device address for TPS65090 PMU */
#define TPS65090_I2C_ADDR	0x48

/* TPS65090 register addresses */
enum {
	REG_IRQ1 = 0,
	REG_CG_CTRL0 = 4,
	REG_CG_STATUS1 = 0xa,
	REG_FET_BASE = 0xe,	/* Not a real register, FETs count from here */
	REG_FET1_CTRL,
	REG_FET2_CTRL,
	REG_FET3_CTRL,
	REG_FET4_CTRL,
	REG_FET5_CTRL,
	REG_FET6_CTRL,
	REG_FET7_CTRL,
	TPS65090_NUM_REGS,
};

enum {
	IRQ1_VBATG = 1 << 3,
	CG_CTRL0_ENC_MASK	= 0x01,

	MAX_FET_NUM	= 7,
	MAX_CTRL_READ_TRIES = 5,

	/* TPS65090 FET_CTRL register values */
	FET_CTRL_TOFET		= 1 << 7,  /* Timeout, startup, overload */
	FET_CTRL_PGFET		= 1 << 4,  /* Power good for FET status */
	FET_CTRL_WAIT		= 3 << 2,  /* Overcurrent timeout max */
	FET_CTRL_ADENFET	= 1 << 1,  /* Enable output auto discharge */
	FET_CTRL_ENFET		= 1 << 0,  /* Enable FET */
};

enum {
	/* Status register fields */
	TPS65090_ST1_OTC	= 1 << 0,
	TPS65090_ST1_OCC	= 1 << 1,
	TPS65090_ST1_STATE_SHIFT = 4,
	TPS65090_ST1_STATE_MASK	= 0xf << TPS65090_ST1_STATE_SHIFT,
};

/* Drivers name */
#define TPS65090_FET_DRIVER	"tps65090_fet"

#endif /* __TPS65090_PMIC_H_ */

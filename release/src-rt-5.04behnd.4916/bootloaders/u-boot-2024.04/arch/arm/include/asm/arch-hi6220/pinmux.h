/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */

#ifndef __ASM_ARM_ARCH_PINMUX_H
#define __ASM_ARM_ARCH_PINMUX_H

#include "periph.h"


/* iomg bit definition */
#define MUX_M0          0
#define MUX_M1          1
#define MUX_M2          2
#define MUX_M3          3
#define MUX_M4          4
#define MUX_M5          5
#define MUX_M6          6
#define MUX_M7          7

/* iocg bit definition */
#define PULL_MASK       (3)
#define PULL_DIS        (0)
#define PULL_UP         (1 << 0)
#define PULL_DOWN       (1 << 1)

/* drive strength definition */
#define DRIVE_MASK      (7 << 4)
#define DRIVE1_02MA     (0 << 4)
#define DRIVE1_04MA     (1 << 4)
#define DRIVE1_08MA     (2 << 4)
#define DRIVE1_10MA     (3 << 4)
#define DRIVE2_02MA     (0 << 4)
#define DRIVE2_04MA     (1 << 4)
#define DRIVE2_08MA     (2 << 4)
#define DRIVE2_10MA     (3 << 4)
#define DRIVE3_04MA     (0 << 4)
#define DRIVE3_08MA     (1 << 4)
#define DRIVE3_12MA     (2 << 4)
#define DRIVE3_16MA     (3 << 4)
#define DRIVE3_20MA     (4 << 4)
#define DRIVE3_24MA     (5 << 4)
#define DRIVE3_32MA     (6 << 4)
#define DRIVE3_40MA     (7 << 4)
#define DRIVE4_02MA     (0 << 4)
#define DRIVE4_04MA     (2 << 4)
#define DRIVE4_08MA     (4 << 4)
#define DRIVE4_10MA     (6 << 4)

#define HI6220_PINMUX0_BASE 0xf7010000
#define HI6220_PINMUX1_BASE 0xf7010800

#ifndef	__ASSEMBLY__

/* maybe more registers, but highest used is 123 */
#define REG_NUM 123

struct hi6220_pinmux0_regs {
	uint32_t	iomg[REG_NUM];
};

struct hi6220_pinmux1_regs {
	uint32_t	iocfg[REG_NUM];
};

#endif

/**
 * Configures the pinmux for a particular peripheral.
 *
 * This function will configure the peripheral pinmux along with
 * pull-up/down and drive strength.
 *
 * @param peripheral	peripheral to be configured
 * Return: 0 if ok, -1 on error (e.g. unsupported peripheral)
 */
int hi6220_pinmux_config(int peripheral);

#endif

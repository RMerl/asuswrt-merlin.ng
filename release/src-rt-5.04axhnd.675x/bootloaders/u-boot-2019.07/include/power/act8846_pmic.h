/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _PMIC_ACT8846_H_
#define _PMIC_ACT8846_H_

#include <asm/gpio.h>

#define ACT8846_NUM_OF_REGS	12

#define BUCK_VOL_MASK 0x3f
#define LDO_VOL_MASK 0x3f

#define BUCK_EN_MASK 0x80
#define LDO_EN_MASK 0x80

#define VOL_MIN_IDX 0x00
#define VOL_MAX_IDX 0x3f

struct  act8846_reg_table {
	char	*name;
	char	reg_ctl;
	char	reg_vol;
};

struct pmic_act8846 {
	struct pmic *pmic;
	int node;	/*device tree node*/
	struct gpio_desc pwr_hold;
	struct udevice *dev;
};

#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 * Peng Fan <Peng.Fan@freescale.com>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>

int power_pfuze3000_init(unsigned char bus)
{
	static const char name[] = "PFUZE3000";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PFUZE3000_NUM_OF_REGS;
	p->hw.i2c.addr = CONFIG_POWER_PFUZE3000_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

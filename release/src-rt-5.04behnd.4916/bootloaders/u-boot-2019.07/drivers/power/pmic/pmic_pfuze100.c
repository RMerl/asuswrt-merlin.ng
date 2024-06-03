// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>

int power_pfuze100_init(unsigned char bus)
{
	static const char name[] = "PFUZE100";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PFUZE100_NUM_OF_REGS;
	p->hw.i2c.addr = CONFIG_POWER_PFUZE100_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

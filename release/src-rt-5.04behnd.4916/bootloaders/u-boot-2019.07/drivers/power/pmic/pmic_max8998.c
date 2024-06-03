// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2011 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <power/pmic.h>
#include <power/max8998_pmic.h>
#include <errno.h>

int pmic_init(unsigned char bus)
{
	static const char name[] = "MAX8998_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	puts("Board PMIC init\n");

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.addr = MAX8998_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

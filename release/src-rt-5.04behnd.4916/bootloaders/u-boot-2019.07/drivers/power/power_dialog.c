// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2011 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <power/pmic.h>
#include <dialog_pmic.h>
#include <errno.h>

int pmic_dialog_init(unsigned char bus)
{
	static const char name[] = "DIALOG_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->number_of_regs = DIALOG_NUM_OF_REGS;

	p->interface = PMIC_I2C;
	p->hw.i2c.addr = CONFIG_SYS_DIALOG_PMIC_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

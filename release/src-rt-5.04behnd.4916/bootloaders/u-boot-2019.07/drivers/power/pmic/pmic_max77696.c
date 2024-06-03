// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Freescale Semiconductor, Inc.
 *  Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/max77696_pmic.h>

int power_max77696_init(unsigned char bus)
{
	static const char name[] = "MAX77696";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.addr = CONFIG_POWER_MAX77696_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

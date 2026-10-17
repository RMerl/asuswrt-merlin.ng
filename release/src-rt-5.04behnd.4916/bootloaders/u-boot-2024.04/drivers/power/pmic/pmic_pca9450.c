// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pca9450.h>

static const char pca9450_name[] = "PCA9450";

int power_pca9450_init(unsigned char bus, unsigned char addr)
{
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = pca9450_name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PCA9450_REG_NUM;
	p->hw.i2c.addr = addr;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

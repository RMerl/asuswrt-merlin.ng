// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Hou Zhiqiang <Zhiqiang.Hou@freescale.com>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/mc34vr500_pmic.h>

int power_mc34vr500_init(unsigned char bus)
{
	static const char name[] = "MC34VR500";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = MC34VR500_NUM_OF_REGS;
	p->hw.i2c.addr = MC34VR500_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

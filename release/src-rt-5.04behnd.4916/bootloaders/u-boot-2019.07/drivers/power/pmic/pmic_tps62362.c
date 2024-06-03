// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Texas Instruments Incorporated -  http://www.ti.com
 * Author: Felipe Balbi <balbi@ti.com>
 */

#include <common.h>
#include <i2c.h>
#include <linux/errno.h>
#include <power/pmic.h>
#include <power/tps62362.h>

#ifdef CONFIG_DM_I2C
struct udevice *tps62362_dev __attribute__((section(".data"))) = NULL;
#endif

/**
 * tps62362_voltage_update() - Function to change a voltage level, as this
 *			       is a multi-step process.
 * @reg:	Register address to write to
 * @volt_sel:	Voltage register value to write
 * @return:	0 on success, 1 on failure
 */
int tps62362_voltage_update(unsigned char reg, unsigned char volt_sel)
{
	if (reg > TPS62362_NUM_REGS)
		return 1;

#ifndef CONFIG_DM_I2C
	return i2c_write(TPS62362_I2C_ADDR, reg, 1, &volt_sel, 1);
#else
	if (!tps62362_dev)
		return -ENODEV;
	return dm_i2c_reg_write(tps62362_dev, reg, volt_sel);
#endif
}

#ifndef CONFIG_DM_I2C
int power_tps62362_init(unsigned char bus)
{
	static const char name[] = "TPS62362";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = TPS62362_NUM_REGS;
	p->hw.i2c.addr = TPS62362_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}
#else
int power_tps62362_init(unsigned char bus)
{
	struct udevice *dev = NULL;
	int rc;

	rc = i2c_get_chip_for_busnum(bus, TPS62362_I2C_ADDR, 1, &dev);
	if (rc)
		return rc;
	tps62362_dev = dev;
	return 0;
}
#endif

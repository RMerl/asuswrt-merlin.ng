// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Samsung Electronics
 * Piotr Wilczek <p.wilczek@samsung.com>
 */

#include <common.h>
#include <power/pmic.h>
#include <power/max77693_pmic.h>
#include <i2c.h>
#include <errno.h>

static int max77693_charger_state(struct pmic *p, int state, int current)
{
	unsigned int val;

	if (pmic_probe(p))
		return -ENODEV;

	/* unlock write capability */
	val = MAX77693_CHG_UNLOCK;
	pmic_reg_write(p, MAX77693_CHG_CNFG_06, val);

	if (state == PMIC_CHARGER_DISABLE) {
		puts("Disable the charger.\n");
		pmic_reg_read(p, MAX77693_CHG_CNFG_00, &val);
		val &= ~0x01;
		pmic_reg_write(p, MAX77693_CHG_CNFG_00, val);
		return -ENOTSUPP;
	}

	if (current < CHARGER_MIN_CURRENT || current > CHARGER_MAX_CURRENT) {
		printf("%s: Wrong charge current: %d [mA]\n",
		       __func__, current);
		return -EINVAL;
	}

	/* set charging current */
	pmic_reg_read(p, MAX77693_CHG_CNFG_02, &val);
	val &= ~MAX77693_CHG_CC;
	val |= current * 10 / 333;	/* 0.1A/3 steps */
	pmic_reg_write(p, MAX77693_CHG_CNFG_02, val);

	/* enable charging */
	val = MAX77693_CHG_MODE_ON;
	pmic_reg_write(p, MAX77693_CHG_CNFG_00, val);

	/* check charging current */
	pmic_reg_read(p, MAX77693_CHG_CNFG_02, &val);
	val &= 0x3f;
	printf("Enable the charger @ %d [mA]\n", val * 333 / 10);

	return 0;
}

static int max77693_charger_bat_present(struct pmic *p)
{
	unsigned int val;

	if (pmic_probe(p))
		return -ENODEV;

	pmic_reg_read(p, MAX77693_CHG_INT_OK, &val);

	return !(val & MAX77693_CHG_DETBAT);
}

static struct power_chrg power_chrg_pmic_ops = {
	.chrg_bat_present = max77693_charger_bat_present,
	.chrg_state = max77693_charger_state,
};

int pmic_init_max77693(unsigned char bus)
{
	static const char name[] = "MAX77693_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board PMIC init\n");

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.addr = MAX77693_PMIC_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	p->chrg = &power_chrg_pmic_ops;

	return 0;
}

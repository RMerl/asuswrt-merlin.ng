// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <power/pmic.h>
#include <power/power_chrg.h>
#include <power/max8997_muic.h>
#include <i2c.h>
#include <errno.h>

static int power_chrg_get_type(struct pmic *p)
{
	unsigned int val;
	unsigned char charge_type, charger;

	if (pmic_probe(p))
		return CHARGER_NO;

	pmic_reg_read(p, MAX8997_MUIC_STATUS2, &val);
	charge_type = val & MAX8997_MUIC_CHG_MASK;

	switch (charge_type) {
	case MAX8997_MUIC_CHG_NO:
		charger = CHARGER_NO;
		break;
	case MAX8997_MUIC_CHG_USB:
	case MAX8997_MUIC_CHG_USB_D:
		charger = CHARGER_USB;
		break;
	case MAX8997_MUIC_CHG_TA:
	case MAX8997_MUIC_CHG_TA_1A:
		charger = CHARGER_TA;
		break;
	case MAX8997_MUIC_CHG_TA_500:
		charger = CHARGER_TA_500;
		break;
	default:
		charger = CHARGER_UNKNOWN;
		break;
	}

	return charger;
}

static struct power_chrg power_chrg_muic_ops = {
	.chrg_type = power_chrg_get_type,
};

int power_muic_init(unsigned int bus)
{
	static const char name[] = "MAX8997_MUIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board Micro USB Interface Controller init\n");

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = MUIC_NUM_OF_REGS;
	p->hw.i2c.addr = MAX8997_MUIC_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	p->chrg = &power_chrg_muic_ops;
	return 0;
}

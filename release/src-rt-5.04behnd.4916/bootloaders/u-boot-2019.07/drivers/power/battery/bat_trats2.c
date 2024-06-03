// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013 Samsung Electronics
 *  Piotr Wilczek <p.wilczek@samsung.com>
 */

#include <common.h>
#include <power/pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <errno.h>

static struct battery battery_trats;

static int power_battery_charge(struct pmic *bat)
{
	struct power_battery *p_bat = bat->pbat;

	if (bat->chrg->chrg_state(p_bat->chrg, PMIC_CHARGER_ENABLE, 450))
		return -EINVAL;

	return 0;
}

static int power_battery_init_trats2(struct pmic *bat_,
				    struct pmic *fg_,
				    struct pmic *chrg_,
				    struct pmic *muic_)
{
	bat_->pbat->fg = fg_;
	bat_->pbat->chrg = chrg_;
	bat_->pbat->muic = muic_;

	bat_->fg = fg_->fg;
	bat_->chrg = chrg_->chrg;
	bat_->chrg->chrg_type = muic_->chrg->chrg_type;
	return 0;
}

static struct power_battery power_bat_trats2 = {
	.bat = &battery_trats,
	.battery_init = power_battery_init_trats2,
	.battery_charge = power_battery_charge,
};

int power_bat_init(unsigned char bus)
{
	static const char name[] = "BAT_TRATS2";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board BAT init\n");

	p->interface = PMIC_NONE;
	p->name = name;
	p->bus = bus;

	p->pbat = &power_bat_trats2;
	return 0;
}

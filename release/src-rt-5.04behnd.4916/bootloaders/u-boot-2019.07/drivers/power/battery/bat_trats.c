// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <console.h>
#include <power/pmic.h>
#include <power/battery.h>
#include <power/max8997_pmic.h>
#include <errno.h>

static struct battery battery_trats;

static int power_battery_charge(struct pmic *bat)
{
	struct power_battery *p_bat = bat->pbat;
	struct battery *battery = p_bat->bat;
	int k;

	if (bat->chrg->chrg_state(p_bat->chrg, PMIC_CHARGER_ENABLE, 450))
		return -1;

	for (k = 0; bat->chrg->chrg_bat_present(p_bat->chrg) &&
		     bat->chrg->chrg_type(p_bat->muic) &&
		     battery->state_of_chrg < 100; k++) {
		udelay(2000000);
		if (!(k % 5))
			puts(".");
		bat->fg->fg_battery_update(p_bat->fg, bat);

		if (k == 200) {
			debug(" %d [V]", battery->voltage_uV);
			puts("\n");
			k = 0;
		}

		if (ctrlc()) {
			printf("\nCharging disabled on request.\n");
			goto exit;
		}
	}
 exit:
	bat->chrg->chrg_state(p_bat->chrg, PMIC_CHARGER_DISABLE, 0);

	return 0;
}

static int power_battery_init_trats(struct pmic *bat_,
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

static struct power_battery power_bat_trats = {
	.bat = &battery_trats,
	.battery_init = power_battery_init_trats,
	.battery_charge = power_battery_charge,
};

int power_bat_init(unsigned char bus)
{
	static const char name[] = "BAT_TRATS";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board BAT init\n");

	p->interface = PMIC_NONE;
	p->name = name;
	p->bus = bus;

	p->pbat = &power_bat_trats;
	return 0;
}

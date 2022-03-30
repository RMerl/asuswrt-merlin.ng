// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 */
#include <common.h>
#include <command.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

static u8 axp152_mvolt_to_target(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp_set_dcdc2(unsigned int mvolt)
{
	int rc;
	u8 current, target;

	target = axp152_mvolt_to_target(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = pmic_bus_read(AXP152_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != target) {
		if (current < target)
			current++;
		else
			current--;
		rc = pmic_bus_write(AXP152_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}
	return rc;
}

int axp_set_dcdc3(unsigned int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 50);

	return pmic_bus_write(AXP152_DCDC3_VOLTAGE, target);
}

int axp_set_dcdc4(unsigned int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 25);

	return pmic_bus_write(AXP152_DCDC4_VOLTAGE, target);
}

int axp_set_aldo2(unsigned int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 100);

	return pmic_bus_write(AXP152_LDO2_VOLTAGE, target);
}

int axp_init(void)
{
	u8 ver;
	int rc;

	rc = pmic_bus_init();
	if (rc)
		return rc;

	rc = pmic_bus_read(AXP152_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	if (ver != 0x05)
		return -EINVAL;

	return 0;
}

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmic_bus_write(AXP152_SHUTDOWN, AXP152_POWEROFF);

	/* infinite loop during shutdown */
	while (1) {}

	/* not reached */
	return 0;
}

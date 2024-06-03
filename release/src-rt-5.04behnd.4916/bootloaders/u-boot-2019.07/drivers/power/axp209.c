// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 */

#include <common.h>
#include <command.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

#ifdef CONFIG_AXP_ALDO3_VOLT_SLOPE_08
#  define AXP209_VRC_SLOPE AXP209_VRC_LDO3_800uV_uS
#endif
#ifdef CONFIG_AXP_ALDO3_VOLT_SLOPE_16
#  define AXP209_VRC_SLOPE AXP209_VRC_LDO3_1600uV_uS
#endif
#if defined CONFIG_AXP_ALDO3_VOLT_SLOPE_NONE || !defined AXP209_VRC_SLOPE
#  define AXP209_VRC_SLOPE 0x00
#endif

static u8 axp209_mvolt_to_cfg(int mvolt, int min, int max, int div)
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
	u8 cfg, current;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP209_OUTPUT_CTRL,
					AXP209_OUTPUT_CTRL_DCDC2);

	rc = pmic_bus_setbits(AXP209_OUTPUT_CTRL, AXP209_OUTPUT_CTRL_DCDC2);
	if (rc)
		return rc;

	cfg = axp209_mvolt_to_cfg(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = pmic_bus_read(AXP209_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != cfg) {
		if (current < cfg)
			current++;
		else
			current--;

		rc = pmic_bus_write(AXP209_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}

	return rc;
}

int axp_set_dcdc3(unsigned int mvolt)
{
	u8 cfg = axp209_mvolt_to_cfg(mvolt, 700, 3500, 25);
	int rc;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP209_OUTPUT_CTRL,
					AXP209_OUTPUT_CTRL_DCDC3);

	rc = pmic_bus_write(AXP209_DCDC3_VOLTAGE, cfg);
	if (rc)
		return rc;

	return pmic_bus_setbits(AXP209_OUTPUT_CTRL, AXP209_OUTPUT_CTRL_DCDC3);
}

int axp_set_aldo2(unsigned int mvolt)
{
	int rc;
	u8 cfg, reg;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP209_OUTPUT_CTRL,
					AXP209_OUTPUT_CTRL_LDO2);

	cfg = axp209_mvolt_to_cfg(mvolt, 1800, 3300, 100);

	rc = pmic_bus_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	reg |= AXP209_LDO24_LDO2_SET(reg, cfg);
	rc = pmic_bus_write(AXP209_LDO24_VOLTAGE, reg);
	if (rc)
		return rc;

	return pmic_bus_setbits(AXP209_OUTPUT_CTRL, AXP209_OUTPUT_CTRL_LDO2);
}

int axp_set_aldo3(unsigned int mvolt)
{
	u8 cfg;
	int rc;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP209_OUTPUT_CTRL,
					AXP209_OUTPUT_CTRL_LDO3);

	/*
	 * Some boards have trouble reaching the target voltage without causing
	 * great inrush currents. To prevent this, boards can enable a certain
	 * slope to ramp up voltage. Note, this only works when changing an
	 * already active power rail. When toggling power on, the AXP ramps up
	 * steeply at 0.0167 V/uS.
	 */
	rc = pmic_bus_read(AXP209_VRC_DCDC2_LDO3, &cfg);
	cfg = AXP209_VRC_LDO3_SLOPE_SET(cfg, AXP209_VRC_SLOPE);
	rc |= pmic_bus_write(AXP209_VRC_DCDC2_LDO3, cfg);

	if (rc)
		return rc;

#ifdef CONFIG_AXP_ALDO3_INRUSH_QUIRK
	/*
	 * On some boards, LDO3 has a too big capacitor installed. When
	 * turning on LDO3, this causes the AXP209 to shutdown on
	 * voltages over 1.9 volt. As a workaround, we enable LDO3
	 * first with the lowest possible voltage. If this still causes
	 * high inrush currents, the voltage slope should be increased.
	 */
	rc = pmic_bus_read(AXP209_OUTPUT_CTRL, &cfg);
	if (rc)
		return rc;

	if (!(cfg & AXP209_OUTPUT_CTRL_LDO3)) {
		rc = pmic_bus_write(AXP209_LDO3_VOLTAGE, 0x0); /* 0.7 Volt */
		mdelay(1);
		rc |= pmic_bus_setbits(AXP209_OUTPUT_CTRL,
				       AXP209_OUTPUT_CTRL_LDO3);

		if (rc)
			return rc;
	}
#endif

	if (mvolt == -1) {
		cfg = AXP209_LDO3_VOLTAGE_FROM_LDO3IN;
	} else {
		cfg = axp209_mvolt_to_cfg(mvolt, 700, 3500, 25);
		cfg = AXP209_LDO3_VOLTAGE_SET(cfg);
	}

	rc = pmic_bus_write(AXP209_LDO3_VOLTAGE, cfg);
	if (rc)
		return rc;

	return pmic_bus_setbits(AXP209_OUTPUT_CTRL, AXP209_OUTPUT_CTRL_LDO3);
}

int axp_set_aldo4(unsigned int mvolt)
{
	int rc;
	static const unsigned int vindex[] = {
		1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2500,
		2700, 2800, 3000, 3100, 3200, 3300
	};
	u8 cfg, reg;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP209_OUTPUT_CTRL,
					AXP209_OUTPUT_CTRL_LDO4);

	/* Translate mvolt to register cfg value, requested <= selected */
	for (cfg = 15; vindex[cfg] > mvolt && cfg > 0; cfg--);

	rc = pmic_bus_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	reg |= AXP209_LDO24_LDO4_SET(reg, cfg);
	rc = pmic_bus_write(AXP209_LDO24_VOLTAGE, reg);
	if (rc)
		return rc;

	return pmic_bus_setbits(AXP209_OUTPUT_CTRL, AXP209_OUTPUT_CTRL_LDO4);
}

int axp_init(void)
{
	u8 ver;
	int i, rc;

	rc = pmic_bus_init();
	if (rc)
		return rc;

	rc = pmic_bus_read(AXP209_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	if ((ver & AXP209_CHIP_VERSION_MASK) != 0x1)
		return -EINVAL;

	/* Mask all interrupts */
	for (i = AXP209_IRQ_ENABLE1; i <= AXP209_IRQ_ENABLE5; i++) {
		rc = pmic_bus_write(i, 0);
		if (rc)
			return rc;
	}

	/*
	 * Turn off LDOIO regulators / tri-state GPIO pins, when rebooting
	 * from android these are sometimes on.
	 */
	rc = pmic_bus_write(AXP_GPIO0_CTRL, AXP_GPIO_CTRL_INPUT);
	if (rc)
		return rc;

	rc = pmic_bus_write(AXP_GPIO1_CTRL, AXP_GPIO_CTRL_INPUT);
	if (rc)
		return rc;

	rc = pmic_bus_write(AXP_GPIO2_CTRL, AXP_GPIO_CTRL_INPUT);
	if (rc)
		return rc;

	return 0;
}

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmic_bus_write(AXP209_SHUTDOWN, AXP209_POWEROFF);

	/* infinite loop during shutdown */
	while (1) {}

	/* not reached */
	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * AXP809 driver based on AXP221 driver
 *
 *
 * (C) Copyright 2016 Chen-Yu Tsai <wens@csie.org>
 *
 * Based on axp221.c
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

static u8 axp809_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp_set_dcdc1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 1600, 3400, 100);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
					AXP809_OUTPUT_CTRL1_DCDC1_EN);

	ret = pmic_bus_write(AXP809_DCDC1_CTRL, cfg);
	if (ret)
		return ret;

	ret = pmic_bus_setbits(AXP809_OUTPUT_CTRL2,
			       AXP809_OUTPUT_CTRL2_DC1SW_EN);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_DCDC1_EN);
}

int axp_set_dcdc2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 600, 1540, 20);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
					AXP809_OUTPUT_CTRL1_DCDC2_EN);

	ret = pmic_bus_write(AXP809_DCDC2_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_DCDC2_EN);
}

int axp_set_dcdc3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 600, 1860, 20);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
					AXP809_OUTPUT_CTRL1_DCDC3_EN);

	ret = pmic_bus_write(AXP809_DCDC3_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_DCDC3_EN);
}

int axp_set_dcdc4(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 600, 1540, 20);

	if (mvolt >= 1540)
		cfg = 0x30 + axp809_mvolt_to_cfg(mvolt, 1800, 2600, 100);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
					AXP809_OUTPUT_CTRL1_DCDC4_EN);

	ret = pmic_bus_write(AXP809_DCDC5_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_DCDC4_EN);
}

int axp_set_dcdc5(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 1000, 2550, 50);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
					AXP809_OUTPUT_CTRL1_DCDC5_EN);

	ret = pmic_bus_write(AXP809_DCDC5_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_DCDC5_EN);
}

int axp_set_aldo(int aldo_num, unsigned int mvolt)
{
	int ret;
	u8 cfg;

	if (aldo_num < 1 || aldo_num > 3)
		return -EINVAL;

	if (mvolt == 0 && aldo_num == 3)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL2,
					AXP809_OUTPUT_CTRL2_ALDO3_EN);
	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_ALDO1_EN << (aldo_num - 1));

	cfg = axp809_mvolt_to_cfg(mvolt, 700, 3300, 100);
	ret = pmic_bus_write(AXP809_ALDO1_CTRL + (aldo_num - 1), cfg);
	if (ret)
		return ret;

	if (aldo_num == 3)
		return pmic_bus_setbits(AXP809_OUTPUT_CTRL2,
					AXP809_OUTPUT_CTRL2_ALDO3_EN);
	return pmic_bus_setbits(AXP809_OUTPUT_CTRL1,
				AXP809_OUTPUT_CTRL1_ALDO1_EN << (aldo_num - 1));
}

/* TODO: re-work other AXP drivers to consolidate ALDO functions. */
int axp_set_aldo1(unsigned int mvolt)
{
	return axp_set_aldo(1, mvolt);
}

int axp_set_aldo2(unsigned int mvolt)
{
	return axp_set_aldo(2, mvolt);
}

int axp_set_aldo3(unsigned int mvolt)
{
	return axp_set_aldo(3, mvolt);
}

int axp_set_dldo(int dldo_num, unsigned int mvolt)
{
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 700, 3300, 100);
	int ret;

	if (dldo_num < 1 || dldo_num > 2)
		return -EINVAL;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL2,
				AXP809_OUTPUT_CTRL2_DLDO1_EN << (dldo_num - 1));

	if (dldo_num == 1 && mvolt > 3300)
		cfg += 1 + axp809_mvolt_to_cfg(mvolt, 3400, 4200, 200);
	ret = pmic_bus_write(AXP809_DLDO1_CTRL + (dldo_num - 1), cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL2,
				AXP809_OUTPUT_CTRL2_DLDO1_EN << (dldo_num - 1));
}

int axp_set_eldo(int eldo_num, unsigned int mvolt)
{
	int ret;
	u8 cfg = axp809_mvolt_to_cfg(mvolt, 700, 3300, 100);

	if (eldo_num < 1 || eldo_num > 3)
		return -EINVAL;

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP809_OUTPUT_CTRL2,
				AXP809_OUTPUT_CTRL2_ELDO1_EN << (eldo_num - 1));

	ret = pmic_bus_write(AXP809_ELDO1_CTRL + (eldo_num - 1), cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP809_OUTPUT_CTRL2,
				AXP809_OUTPUT_CTRL2_ELDO1_EN << (eldo_num - 1));
}

int axp_set_sw(bool on)
{
	if (on)
		return pmic_bus_setbits(AXP809_OUTPUT_CTRL2,
					AXP809_OUTPUT_CTRL2_SWOUT_EN);

	return pmic_bus_clrbits(AXP809_OUTPUT_CTRL2,
				AXP809_OUTPUT_CTRL2_SWOUT_EN);
}

int axp_init(void)
{
	return pmic_bus_init();
}

int do_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pmic_bus_write(AXP809_SHUTDOWN, AXP809_SHUTDOWN_POWEROFF);

	/* infinite loop during shutdown */
	while (1) {}

	/* not reached */
	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Based on Rockchip's drivers/power/pmic/pmic_act8846.c:
 * Copyright (C) 2012 rockchips
 * zyw <zyw@rock-chips.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/act8846_pmic.h>
#include <power/pmic.h>
#include <power/regulator.h>

static const u16 voltage_map[] = {
	600, 625, 650, 675, 700, 725, 750, 775,
	800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175,
	1200, 1250, 1300, 1350, 1400, 1450, 1500, 1550,
	1600, 1650, 1700, 1750, 1800, 1850, 1900, 1950,
	2000, 2050, 2100, 2150, 2200, 2250, 2300, 2350,
	2400, 2500, 2600, 2700, 2800, 2900, 3000, 3100,
	3200, 3300, 3400, 3500, 3600, 3700, 3800, 3900,
};

enum {
	REG_SYS0,
	REG_SYS1,
	REG1_VOL	= 0x10,
	REG1_CTL	= 0X11,
	REG2_VOL0	= 0x20,
	REG2_VOL1,
	REG2_CTL,
	REG3_VOL0	= 0x30,
	REG3_VOL1,
	REG3_CTL,
	REG4_VOL0	= 0x40,
	REG4_VOL1,
	REG4_CTL,
	REG5_VOL	= 0x50,
	REG5_CTL,
	REG6_VOL	= 0X58,
	REG6_CTL,
	REG7_VOL	= 0x60,
	REG7_CTL,
	REG8_VOL	= 0x68,
	REG8_CTL,
	REG9_VOL	= 0x70,
	REG9_CTL,
	REG10_VOL	= 0x80,
	REG10_CTL,
	REG11_VOL	= 0x90,
	REG11_CTL,
	REG12_VOL	= 0xa0,
	REG12_CTL,
	REG13		= 0xb1,
};

static const u8 addr_vol[] = {
	0, REG1_VOL, REG2_VOL0, REG3_VOL0, REG4_VOL0,
	REG5_VOL, REG6_VOL, REG7_VOL, REG8_VOL, REG9_VOL,
	REG10_VOL, REG11_VOL, REG12_VOL,
};

static const u8 addr_ctl[] = {
	0, REG1_CTL, REG2_CTL, REG3_CTL, REG4_CTL,
	REG5_CTL, REG6_CTL, REG7_CTL, REG8_CTL, REG9_CTL,
	REG10_CTL, REG11_CTL, REG12_CTL,
};

static int check_volt_table(const u16 *volt_table, int uvolt)
{
	int i;

	for (i = VOL_MIN_IDX; i < VOL_MAX_IDX; i++) {
		if (uvolt <= (volt_table[i] * 1000))
			return i;
	}
	return -EINVAL;
}

static int reg_get_value(struct udevice *dev)
{
	int reg = dev->driver_data;
	int ret;

	ret = pmic_reg_read(dev->parent, addr_vol[reg]);
	if (ret < 0)
		return ret;

	return voltage_map[ret & LDO_VOL_MASK] * 1000;
}

static int reg_set_value(struct udevice *dev, int uvolt)
{
	int reg = dev->driver_data;
	int val;

	val = check_volt_table(voltage_map, uvolt);
	if (val < 0)
		return val;

	return pmic_clrsetbits(dev->parent, addr_vol[reg], LDO_VOL_MASK, val);
}

static int reg_set_enable(struct udevice *dev, bool enable)
{
	int reg = dev->driver_data;

	return pmic_clrsetbits(dev->parent, addr_ctl[reg], LDO_EN_MASK,
			       enable ? LDO_EN_MASK : 0);
}

static int reg_get_enable(struct udevice *dev)
{
	int reg = dev->driver_data;
	int ret;

	ret = pmic_reg_read(dev->parent, addr_ctl[reg]);
	if (ret < 0)
		return ret;

	return ret & LDO_EN_MASK ? true : false;
}

static int act8846_reg_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int reg = dev->driver_data;

	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = reg <= 4 ? REGULATOR_TYPE_BUCK : REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops act8846_reg_ops = {
	.get_value  = reg_get_value,
	.set_value  = reg_set_value,
	.get_enable = reg_get_enable,
	.set_enable = reg_set_enable,
};

U_BOOT_DRIVER(act8846_buck) = {
	.name = "act8846_reg",
	.id = UCLASS_REGULATOR,
	.ops = &act8846_reg_ops,
	.probe = act8846_reg_probe,
};

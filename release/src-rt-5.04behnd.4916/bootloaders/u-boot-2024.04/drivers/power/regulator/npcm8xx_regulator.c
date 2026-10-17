// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

#define REG_VSRCR	0xf08000e8	/* Voltage Supply Control Register */

/* Supported voltage levels (uV) */
static const u32 volts_type1[] = { 3300000, 1800000 };
static const u32 volts_type2[] = { 1000000, 1800000 };
#define VOLT_LEV0	0
#define VOLT_LEV1	1

struct volt_supply {
	char *name;
	const u32 *volts;
	u32 reg_shift;	/* Register bit offset for setting voltage */
};

static const struct volt_supply npcm8xx_volt_supps[] = {
	{"v1", volts_type1, 0},
	{"v2", volts_type1, 1},
	{"v3", volts_type1, 2},
	{"v4", volts_type1, 3},
	{"v5", volts_type1, 4},
	{"v6", volts_type1, 5},
	{"v7", volts_type1, 6},
	{"v8", volts_type1, 7},
	{"v9", volts_type1, 8},
	{"v10", volts_type1, 9},
	{"v11", volts_type2, 10},
	{"v12", volts_type1, 11},
	{"v13", volts_type1, 12},
	{"v14", volts_type2, 13},
	{"vsif", volts_type1, 14},
	{"vr2", volts_type1, 30},
};

static const struct volt_supply *npcm8xx_volt_supply_get(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(npcm8xx_volt_supps); i++) {
		if (!strcmp(npcm8xx_volt_supps[i].name, name))
			return &npcm8xx_volt_supps[i];
	}

	return NULL;
}

static int npcm8xx_regulator_set_value(struct udevice *dev, int uV)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	const struct volt_supply *supp;
	u32 val, level;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	dev_dbg(dev, "%s set_value: %d\n", uc_pdata->name, uV);
	supp = npcm8xx_volt_supply_get(uc_pdata->name);
	if (!supp)
		return -ENOENT;

	if (uV == supp->volts[VOLT_LEV0])
		level = VOLT_LEV0;
	else if (uV == supp->volts[VOLT_LEV1])
		level = VOLT_LEV1;
	else
		return -EINVAL;

	/* Set voltage level */
	val = readl(REG_VSRCR);
	val &= ~BIT(supp->reg_shift);
	val |= level << supp->reg_shift;
	writel(val, REG_VSRCR);

	return 0;
}

static int npcm8xx_regulator_get_value(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	const struct volt_supply *supp;
	u32 val;

	uc_pdata = dev_get_uclass_plat(dev);
	if (!uc_pdata)
		return -ENXIO;

	supp = npcm8xx_volt_supply_get(uc_pdata->name);
	if (!supp)
		return -ENOENT;

	val = readl(REG_VSRCR) & BIT(supp->reg_shift);

	dev_dbg(dev, "%s get_value: %d\n", uc_pdata->name,
		val ? supp->volts[VOLT_LEV1] : supp->volts[VOLT_LEV0]);

	return val ? supp->volts[VOLT_LEV1] : supp->volts[VOLT_LEV0];
}

static int npcm8xx_regulator_set_enable(struct udevice *dev, bool enable)
{
	/* Always on */
	return 0;
}

static const struct dm_regulator_ops npcm8xx_regulator_ops = {
	.set_value	= npcm8xx_regulator_set_value,
	.get_value	= npcm8xx_regulator_get_value,
	.set_enable	= npcm8xx_regulator_set_enable,
};

static const struct udevice_id npcm8xx_regulator_ids[] = {
	{ .compatible = "regulator-npcm845" },
	{ },
};

U_BOOT_DRIVER(regulator_npcm8xx) = {
	.name = "regulator_npcm845",
	.id = UCLASS_REGULATOR,
	.ops = &npcm8xx_regulator_ops,
	.of_match = npcm8xx_regulator_ids,
};

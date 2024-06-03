// SPDX-License-Identifier:     GPL-2.0+
/*
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <bitfield.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/regulator.h>

/**
 * struct ic_types - definition of fan53555-family devices
 *
 * @die_id: Identifies the DIE_ID (lower nibble of the ID1 register)
 * @die_rev: Identifies the DIE_REV (lower nibble of the ID2 register)
 * @vsel_min: starting voltage (step 0) in uV
 * @vsel_step: increment of the voltage in uV
 *
 * The voltage ramp (i.e. minimum voltage and step) is selected from the
 * combination of 2 nibbles: DIE_ID and DIE_REV.
 *
 * See http://www.onsemi.com/pub/Collateral/FAN53555-D.pdf for details.
 */
static const struct {
	u8 die_id;
	u8 die_rev;
	u32 vsel_min;
	u32 vsel_step;
} ic_types[] = {
	{ 0x0, 0x3, 600000, 10000 },  /* Option 00 */
	{ 0x0, 0xf, 800000, 10000 },  /* Option 13 */
	{ 0x0, 0xc, 600000, 12500 },  /* Option 23 */
	{ 0x1, 0x3, 600000, 10000 },  /* Option 01 */
	{ 0x3, 0x3, 600000, 10000 },  /* Option 03 */
	{ 0x4, 0xf, 603000, 12826 },  /* Option 04 */
	{ 0x5, 0x3, 600000, 10000 },  /* Option 05 */
	{ 0x8, 0x1, 600000, 10000 },  /* Option 08 */
	{ 0x8, 0xf, 600000, 10000 },  /* Option 08 */
	{ 0xc, 0xf, 603000, 12826 },  /* Option 09 */
};

/* I2C-accessible byte-sized registers */
enum {
	/* Voltage setting */
	FAN53555_VSEL0 = 0x00,
	FAN53555_VSEL1,
	/* Control register */
	FAN53555_CONTROL,
	/* IC Type */
	FAN53555_ID1,
	/* IC mask version */
	FAN53555_ID2,
	/* Monitor register */
	FAN53555_MONITOR,
};

struct fan53555_platdata {
	/* Voltage setting register */
	unsigned int vol_reg;
	unsigned int sleep_reg;

};

struct fan53555_priv {
	/* IC Vendor */
	unsigned int vendor;
	/* IC Type and Rev */
	unsigned int die_id;
	unsigned int die_rev;
	/* Voltage range and step(linear) */
	unsigned int vsel_min;
	unsigned int vsel_step;
	/* Voltage slew rate limiting */
	unsigned int slew_rate;
	/* Sleep voltage cache */
	unsigned int sleep_vol_cache;
};

static int fan53555_regulator_ofdata_to_platdata(struct udevice *dev)
{
	struct fan53555_platdata *dev_pdata = dev_get_platdata(dev);
	struct dm_regulator_uclass_platdata *uc_pdata =
		dev_get_uclass_platdata(dev);
	u32 sleep_vsel;

	/* This is a buck regulator */
	uc_pdata->type = REGULATOR_TYPE_BUCK;

	sleep_vsel = dev_read_u32_default(dev, "fcs,suspend-voltage-selector",
					  FAN53555_VSEL1);

	/*
	 * Depending on the device-tree settings, the 'normal mode'
	 * voltage is either controlled by VSEL0 or VSEL1.
	 */
	switch (sleep_vsel) {
	case FAN53555_VSEL0:
		dev_pdata->sleep_reg = FAN53555_VSEL0;
		dev_pdata->vol_reg = FAN53555_VSEL1;
		break;
	case FAN53555_VSEL1:
		dev_pdata->sleep_reg = FAN53555_VSEL1;
		dev_pdata->vol_reg = FAN53555_VSEL0;
		break;
	default:
		pr_err("%s: invalid vsel id %d\n", dev->name, sleep_vsel);
		return -EINVAL;
	}

	return 0;
}

static int fan53555_regulator_get_value(struct udevice *dev)
{
	struct fan53555_platdata *pdata = dev_get_platdata(dev);
	struct fan53555_priv *priv = dev_get_priv(dev);
	int reg;
	int voltage;

	/* We only support a single voltage selector (i.e. 'normal' mode). */
	reg = pmic_reg_read(dev->parent, pdata->vol_reg);
	if (reg < 0)
		return reg;
	voltage = priv->vsel_min + (reg & 0x3f) * priv->vsel_step;

	debug("%s: %d uV\n", __func__, voltage);
	return voltage;
}

static int fan53555_regulator_set_value(struct udevice *dev, int uV)
{
	struct fan53555_platdata *pdata = dev_get_platdata(dev);
	struct fan53555_priv *priv = dev_get_priv(dev);
	u8 vol;

	vol = (uV - priv->vsel_min) / priv->vsel_step;
	debug("%s: uV=%d; writing volume %d: %02x\n",
	      __func__, uV, pdata->vol_reg, vol);

	return pmic_clrsetbits(dev, pdata->vol_reg, GENMASK(6, 0), vol);
}

static int fan53555_voltages_setup(struct udevice *dev)
{
	struct fan53555_priv *priv = dev_get_priv(dev);
	int i;

	/* Init voltage range and step */
	for (i = 0; i < ARRAY_SIZE(ic_types); ++i) {
		if (ic_types[i].die_id != priv->die_id)
			continue;

		if (ic_types[i].die_rev != priv->die_rev)
			continue;

		priv->vsel_min = ic_types[i].vsel_min;
		priv->vsel_step = ic_types[i].vsel_step;

		return 0;
	}

	pr_err("%s: %s: die id %d rev %d not supported!\n",
	       dev->name, __func__, priv->die_id, priv->die_rev);
	return -EINVAL;
}

enum {
	DIE_ID_SHIFT = 0,
	DIE_ID_WIDTH = 4,
	DIE_REV_SHIFT = 0,
	DIE_REV_WIDTH = 4,
};

static int fan53555_probe(struct udevice *dev)
{
	struct fan53555_priv *priv = dev_get_priv(dev);
	int ID1, ID2;

	debug("%s\n", __func__);

	/* read chip ID1 and ID2 (two registers, starting at ID1) */
	ID1 = pmic_reg_read(dev->parent, FAN53555_ID1);
	if (ID1 < 0)
		return ID1;

	ID2 = pmic_reg_read(dev->parent, FAN53555_ID2);
	if (ID2 < 0)
		return ID2;

	/* extract vendor, die_id and die_rev */
	priv->vendor = bitfield_extract(ID1, 5, 3);
	priv->die_id = ID1 & GENMASK(3, 0);
	priv->die_rev = ID2 & GENMASK(3, 0);

	if (fan53555_voltages_setup(dev) < 0)
		return -ENODATA;

	debug("%s: FAN53555 option %d rev %d detected\n",
	      __func__, priv->die_id, priv->die_rev);

	return 0;
}

static const struct dm_regulator_ops fan53555_regulator_ops = {
	.get_value	= fan53555_regulator_get_value,
	.set_value	= fan53555_regulator_set_value,
};

U_BOOT_DRIVER(fan53555_regulator) = {
	.name = "fan53555_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &fan53555_regulator_ops,
	.ofdata_to_platdata = fan53555_regulator_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct fan53555_platdata),
	.priv_auto_alloc_size = sizeof(struct fan53555_priv),
	.probe = fan53555_probe,
};

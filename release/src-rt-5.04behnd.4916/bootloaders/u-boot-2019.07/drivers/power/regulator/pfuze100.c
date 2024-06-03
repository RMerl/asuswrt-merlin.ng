// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/pfuze100_pmic.h>

/**
 * struct pfuze100_regulator_desc - regulator descriptor
 *
 * @name: Identify name for the regulator.
 * @type: Indicates the regulator type.
 * @uV_step: Voltage increase for each selector.
 * @vsel_reg: Register for adjust regulator voltage for normal.
 * @vsel_mask: Mask bit for setting regulator voltage for normal.
 * @stby_reg: Register for adjust regulator voltage for standby.
 * @stby_mask: Mask bit for setting regulator voltage for standby.
 * @volt_table: Voltage mapping table (if table based mapping).
 * @voltage: Current voltage for REGULATOR_TYPE_FIXED type regulator.
 */
struct pfuze100_regulator_desc {
	char *name;
	enum regulator_type type;
	unsigned int uV_step;
	unsigned int vsel_reg;
	unsigned int vsel_mask;
	unsigned int stby_reg;
	unsigned int stby_mask;
	unsigned int *volt_table;
	unsigned int voltage;
};

/**
 * struct pfuze100_regulator_platdata - platform data for pfuze100
 *
 * @desc: Points the description entry of one regulator of pfuze100
 */
struct pfuze100_regulator_platdata {
	struct pfuze100_regulator_desc *desc;
};

#define PFUZE100_FIXED_REG(_name, base, vol)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_FIXED,		\
		.voltage	=	(vol),				\
	}

#define PFUZE100_SW_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_BUCK,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask	=	0x3F,				\
		.stby_reg	=	(base) + PFUZE100_STBY_OFFSET,	\
		.stby_mask	=	0x3F,				\
	}

#define PFUZE100_SWB_REG(_name, base, mask, step, voltages)		\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_BUCK,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base),				\
		.vsel_mask	=	(mask),				\
		.volt_table	=	(voltages),			\
	}

#define PFUZE100_SNVS_REG(_name, base, mask, voltages)			\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_OTHER,		\
		.vsel_reg	=	(base),				\
		.vsel_mask	=	(mask),				\
		.volt_table	=	(voltages),			\
	}

#define PFUZE100_VGEN_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_LDO,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base),				\
		.vsel_mask	=	0xF,				\
		.stby_reg	=	(base),				\
		.stby_mask	=	0x20,				\
	}

#define PFUZE3000_VCC_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_LDO,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base),				\
		.vsel_mask	=	0x3,				\
		.stby_reg	=	(base),				\
		.stby_mask	=	0x20,				\
}

#define PFUZE3000_SW1_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_BUCK,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask	=	0x1F,				\
		.stby_reg	=	(base) + PFUZE100_STBY_OFFSET,	\
		.stby_mask	=	0x1F,				\
	}

#define PFUZE3000_SW2_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_BUCK,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask	=	0x7,				\
		.stby_reg	=	(base) + PFUZE100_STBY_OFFSET,	\
		.stby_mask	=	0x7,				\
	}

#define PFUZE3000_SW3_REG(_name, base, step)				\
	{								\
		.name		=	#_name,				\
		.type		=	REGULATOR_TYPE_BUCK,		\
		.uV_step	=	(step),				\
		.vsel_reg	=	(base) + PFUZE100_VOL_OFFSET,	\
		.vsel_mask	=	0xF,				\
		.stby_reg	=	(base) + PFUZE100_STBY_OFFSET,	\
		.stby_mask	=	0xF,				\
	}

static unsigned int pfuze100_swbst[] = {
	5000000, 5050000, 5100000, 5150000
};

static unsigned int pfuze100_vsnvs[] = {
	1000000, 1100000, 1200000, 1300000, 1500000, 1800000, 3000000, -1
};

static unsigned int pfuze3000_vsnvs[] = {
	-1, -1, -1, -1, -1, -1, 3000000, -1
};

static unsigned int pfuze3000_sw2lo[] = {
	1500000, 1550000, 1600000, 1650000, 1700000, 1750000, 1800000, 1850000
};

/* PFUZE100 */
static struct pfuze100_regulator_desc pfuze100_regulators[] = {
	PFUZE100_SW_REG(sw1ab, PFUZE100_SW1ABVOL, 25000),
	PFUZE100_SW_REG(sw1c, PFUZE100_SW1CVOL, 25000),
	PFUZE100_SW_REG(sw2, PFUZE100_SW2VOL, 25000),
	PFUZE100_SW_REG(sw3a, PFUZE100_SW3AVOL, 25000),
	PFUZE100_SW_REG(sw3b, PFUZE100_SW3BVOL, 25000),
	PFUZE100_SW_REG(sw4, PFUZE100_SW4VOL, 25000),
	PFUZE100_SWB_REG(swbst, PFUZE100_SWBSTCON1, 0x3, 50000, pfuze100_swbst),
	PFUZE100_SNVS_REG(vsnvs, PFUZE100_VSNVSVOL, 0x7, pfuze100_vsnvs),
	PFUZE100_FIXED_REG(vrefddr, PFUZE100_VREFDDRCON, 750000),
	PFUZE100_VGEN_REG(vgen1, PFUZE100_VGEN1VOL, 50000),
	PFUZE100_VGEN_REG(vgen2, PFUZE100_VGEN2VOL, 50000),
	PFUZE100_VGEN_REG(vgen3, PFUZE100_VGEN3VOL, 100000),
	PFUZE100_VGEN_REG(vgen4, PFUZE100_VGEN4VOL, 100000),
	PFUZE100_VGEN_REG(vgen5, PFUZE100_VGEN5VOL, 100000),
	PFUZE100_VGEN_REG(vgen6, PFUZE100_VGEN6VOL, 100000),
};

/* PFUZE200 */
static struct pfuze100_regulator_desc pfuze200_regulators[] = {
	PFUZE100_SW_REG(sw1ab, PFUZE100_SW1ABVOL, 25000),
	PFUZE100_SW_REG(sw2, PFUZE100_SW2VOL, 25000),
	PFUZE100_SW_REG(sw3a, PFUZE100_SW3AVOL, 25000),
	PFUZE100_SW_REG(sw3b, PFUZE100_SW3BVOL, 25000),
	PFUZE100_SWB_REG(swbst, PFUZE100_SWBSTCON1, 0x3, 50000, pfuze100_swbst),
	PFUZE100_SNVS_REG(vsnvs, PFUZE100_VSNVSVOL, 0x7, pfuze100_vsnvs),
	PFUZE100_FIXED_REG(vrefddr, PFUZE100_VREFDDRCON, 750000),
	PFUZE100_VGEN_REG(vgen1, PFUZE100_VGEN1VOL, 50000),
	PFUZE100_VGEN_REG(vgen2, PFUZE100_VGEN2VOL, 50000),
	PFUZE100_VGEN_REG(vgen3, PFUZE100_VGEN3VOL, 100000),
	PFUZE100_VGEN_REG(vgen4, PFUZE100_VGEN4VOL, 100000),
	PFUZE100_VGEN_REG(vgen5, PFUZE100_VGEN5VOL, 100000),
	PFUZE100_VGEN_REG(vgen6, PFUZE100_VGEN6VOL, 100000),
};

/* PFUZE3000 */
static struct pfuze100_regulator_desc pfuze3000_regulators[] = {
	PFUZE3000_SW1_REG(sw1a, PFUZE100_SW1ABVOL, 25000),
	PFUZE3000_SW1_REG(sw1b, PFUZE100_SW1CVOL, 25000),
	PFUZE100_SWB_REG(sw2, PFUZE100_SW2VOL, 0x7, 50000, pfuze3000_sw2lo),
	PFUZE3000_SW3_REG(sw3, PFUZE100_SW3AVOL, 50000),
	PFUZE100_SWB_REG(swbst, PFUZE100_SWBSTCON1, 0x3, 50000, pfuze100_swbst),
	PFUZE100_SNVS_REG(vsnvs, PFUZE100_VSNVSVOL, 0x7, pfuze3000_vsnvs),
	PFUZE100_FIXED_REG(vrefddr, PFUZE100_VREFDDRCON, 750000),
	PFUZE100_VGEN_REG(vldo1, PFUZE100_VGEN1VOL, 100000),
	PFUZE100_VGEN_REG(vldo2, PFUZE100_VGEN2VOL, 50000),
	PFUZE3000_VCC_REG(vccsd, PFUZE100_VGEN3VOL, 150000),
	PFUZE3000_VCC_REG(v33, PFUZE100_VGEN4VOL, 150000),
	PFUZE100_VGEN_REG(vldo3, PFUZE100_VGEN5VOL, 100000),
	PFUZE100_VGEN_REG(vldo4, PFUZE100_VGEN6VOL, 100000),
};

#define MODE(_id, _val, _name) { \
	.id = _id, \
	.register_value = _val, \
	.name = _name, \
}

/* SWx Buck regulator mode */
static struct dm_regulator_mode pfuze_sw_modes[] = {
	MODE(OFF_OFF, OFF_OFF, "OFF_OFF"),
	MODE(PWM_OFF, PWM_OFF, "PWM_OFF"),
	MODE(PFM_OFF, PFM_OFF, "PFM_OFF"),
	MODE(APS_OFF, APS_OFF, "APS_OFF"),
	MODE(PWM_PWM, PWM_PWM, "PWM_PWM"),
	MODE(PWM_APS, PWM_APS, "PWM_APS"),
	MODE(APS_APS, APS_APS, "APS_APS"),
	MODE(APS_PFM, APS_PFM, "APS_PFM"),
	MODE(PWM_PFM, PWM_PFM, "PWM_PFM"),
};

/* Boost Buck regulator mode for normal operation */
static struct dm_regulator_mode pfuze_swbst_modes[] = {
	MODE(SWBST_MODE_OFF, SWBST_MODE_OFF , "SWBST_MODE_OFF"),
	MODE(SWBST_MODE_PFM, SWBST_MODE_PFM, "SWBST_MODE_PFM"),
	MODE(SWBST_MODE_AUTO, SWBST_MODE_AUTO, "SWBST_MODE_AUTO"),
	MODE(SWBST_MODE_APS, SWBST_MODE_APS, "SWBST_MODE_APS"),
};

/* VGENx LDO regulator mode for normal operation */
static struct dm_regulator_mode pfuze_ldo_modes[] = {
	MODE(LDO_MODE_OFF, LDO_MODE_OFF, "LDO_MODE_OFF"),
	MODE(LDO_MODE_ON, LDO_MODE_ON, "LDO_MODE_ON"),
};

static struct pfuze100_regulator_desc *se_desc(struct pfuze100_regulator_desc *desc,
					       int size,
					       const char *name)
{
	int i;

	for (i = 0; i < size; desc++) {
		if (!strcmp(desc->name, name))
			return desc;
		continue;
	}

	return NULL;
}

static int pfuze100_regulator_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct pfuze100_regulator_platdata *plat = dev_get_platdata(dev);
	struct pfuze100_regulator_desc *desc;

	switch (dev_get_driver_data(dev_get_parent(dev))) {
	case PFUZE100:
		desc = se_desc(pfuze100_regulators,
			       ARRAY_SIZE(pfuze100_regulators),
			       dev->name);
		break;
	case PFUZE200:
		desc = se_desc(pfuze200_regulators,
			       ARRAY_SIZE(pfuze200_regulators),
			       dev->name);
		break;
	case PFUZE3000:
		desc = se_desc(pfuze3000_regulators,
			       ARRAY_SIZE(pfuze3000_regulators),
			       dev->name);
		break;
	default:
		debug("Unsupported PFUZE\n");
		return -EINVAL;
	}
	if (!desc) {
		debug("Do not support regulator %s\n", dev->name);
		return -EINVAL;
	}

	plat->desc = desc;
	uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->type = desc->type;
	if (uc_pdata->type == REGULATOR_TYPE_BUCK) {
		if (!strcmp(dev->name, "swbst")) {
			uc_pdata->mode = pfuze_swbst_modes;
			uc_pdata->mode_count = ARRAY_SIZE(pfuze_swbst_modes);
		} else {
			uc_pdata->mode = pfuze_sw_modes;
			uc_pdata->mode_count = ARRAY_SIZE(pfuze_sw_modes);
		}
	} else if (uc_pdata->type == REGULATOR_TYPE_LDO) {
		uc_pdata->mode = pfuze_ldo_modes;
		uc_pdata->mode_count = ARRAY_SIZE(pfuze_ldo_modes);
	} else {
		uc_pdata->mode = NULL;
		uc_pdata->mode_count = 0;
	}

	return 0;
}

static int pfuze100_regulator_mode(struct udevice *dev, int op, int *opmode)
{
	int val;
	struct pfuze100_regulator_platdata *plat = dev_get_platdata(dev);
	struct pfuze100_regulator_desc *desc = plat->desc;

	if (op == PMIC_OP_GET) {
		if (desc->type == REGULATOR_TYPE_BUCK) {
			if (!strcmp(dev->name, "swbst")) {
				val = pmic_reg_read(dev->parent,
						    desc->vsel_reg);
				if (val < 0)
					return val;

				val &= SWBST_MODE_MASK;
				val >>= SWBST_MODE_SHIFT;
				*opmode = val;

				return 0;
			}
			val = pmic_reg_read(dev->parent,
					    desc->vsel_reg +
					    PFUZE100_MODE_OFFSET);
			if (val < 0)
				return val;

			val &= SW_MODE_MASK;
			val >>= SW_MODE_SHIFT;
			*opmode = val;

			return 0;

		} else if (desc->type == REGULATOR_TYPE_LDO) {
			val = pmic_reg_read(dev->parent, desc->vsel_reg);
			if (val < 0)
				return val;

			val &= LDO_MODE_MASK;
			val >>= LDO_MODE_SHIFT;
			*opmode = val;

			return 0;
		} else {
			return -EINVAL;
		}
	}

	if (desc->type == REGULATOR_TYPE_BUCK) {
		if (!strcmp(dev->name, "swbst"))
			return pmic_clrsetbits(dev->parent, desc->vsel_reg,
					       SWBST_MODE_MASK,
					       *opmode << SWBST_MODE_SHIFT);

		val = pmic_clrsetbits(dev->parent,
				       desc->vsel_reg + PFUZE100_MODE_OFFSET,
				       SW_MODE_MASK,
				       *opmode << SW_MODE_SHIFT);

	} else if (desc->type == REGULATOR_TYPE_LDO) {
		val = pmic_clrsetbits(dev->parent, desc->vsel_reg,
				       LDO_MODE_MASK,
				       *opmode << LDO_MODE_SHIFT);
		return val;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int pfuze100_regulator_enable(struct udevice *dev, int op, bool *enable)
{
	int val;
	int ret, on_off;
	struct dm_regulator_uclass_platdata *uc_pdata =
		dev_get_uclass_platdata(dev);

	if (op == PMIC_OP_GET) {
		if (!strcmp(dev->name, "vrefddr")) {
			val = pmic_reg_read(dev->parent, PFUZE100_VREFDDRCON);
			if (val < 0)
				return val;

			if (val & VREFDDRCON_EN)
				*enable = true;
			else
				*enable = false;
			return 0;
		}
		ret = pfuze100_regulator_mode(dev, op, &on_off);
		if (ret)
			return ret;
		switch (on_off) {
		/* OFF_OFF, SWBST_MODE_OFF, LDO_MODE_OFF have same value */
		case OFF_OFF:
			*enable = false;
			break;
		default:
			*enable = true;
			break;
		}
	} else if (op == PMIC_OP_SET) {
		if (!strcmp(dev->name, "vrefddr")) {
			val = pmic_reg_read(dev->parent, PFUZE100_VREFDDRCON);
			if (val < 0)
				return val;

			if (val & VREFDDRCON_EN)
				return 0;
			val |= VREFDDRCON_EN;

			return pmic_reg_write(dev->parent, PFUZE100_VREFDDRCON,
					      val);
		}

		if (uc_pdata->type == REGULATOR_TYPE_LDO) {
			on_off = *enable ? LDO_MODE_ON : LDO_MODE_OFF;
		} else if (uc_pdata->type == REGULATOR_TYPE_BUCK) {
			if (!strcmp(dev->name, "swbst"))
				on_off = *enable ? SWBST_MODE_AUTO :
					SWBST_MODE_OFF;
			else
				on_off = *enable ? APS_PFM : OFF_OFF;
		} else {
			return -EINVAL;
		}

		return pfuze100_regulator_mode(dev, op, &on_off);
	}

	return 0;
}

static int pfuze100_regulator_val(struct udevice *dev, int op, int *uV)
{
	int i;
	int val;
	struct pfuze100_regulator_platdata *plat = dev_get_platdata(dev);
	struct pfuze100_regulator_desc *desc = plat->desc;
	struct dm_regulator_uclass_platdata *uc_pdata =
		dev_get_uclass_platdata(dev);

	if (op == PMIC_OP_GET) {
		*uV = 0;
		if (uc_pdata->type == REGULATOR_TYPE_FIXED) {
			*uV = desc->voltage;
		} else if (desc->volt_table) {
			val = pmic_reg_read(dev->parent, desc->vsel_reg);
			if (val < 0)
				return val;
			val &= desc->vsel_mask;
			*uV = desc->volt_table[val];
		} else {
			if (uc_pdata->min_uV < 0) {
				debug("Need to provide min_uV in dts.\n");
				return -EINVAL;
			}
			val = pmic_reg_read(dev->parent, desc->vsel_reg);
			if (val < 0)
				return val;
			val &= desc->vsel_mask;
			*uV = uc_pdata->min_uV + (int)val * desc->uV_step;
		}

		return 0;
	}

	if (uc_pdata->type == REGULATOR_TYPE_FIXED) {
		debug("Set voltage for REGULATOR_TYPE_FIXED regulator\n");
		return -EINVAL;
	} else if (desc->volt_table) {
		for (i = 0; i <= desc->vsel_mask; i++) {
			if (*uV == desc->volt_table[i])
				break;
		}
		if (i == desc->vsel_mask + 1) {
			debug("Unsupported voltage %u\n", *uV);
			return -EINVAL;
		}

		return pmic_clrsetbits(dev->parent, desc->vsel_reg,
				       desc->vsel_mask, i);
	} else {
		if (uc_pdata->min_uV < 0) {
			debug("Need to provide min_uV in dts.\n");
			return -EINVAL;
		}
		return pmic_clrsetbits(dev->parent, desc->vsel_reg,
				       desc->vsel_mask,
				       (*uV - uc_pdata->min_uV) / desc->uV_step);
	}

	return 0;
}

static int pfuze100_regulator_get_value(struct udevice *dev)
{
	int uV;
	int ret;

	ret = pfuze100_regulator_val(dev, PMIC_OP_GET, &uV);
	if (ret)
		return ret;

	return uV;
}

static int pfuze100_regulator_set_value(struct udevice *dev, int uV)
{
	return pfuze100_regulator_val(dev, PMIC_OP_SET, &uV);
}

static int pfuze100_regulator_get_enable(struct udevice *dev)
{
	int ret;
	bool enable = false;

	ret = pfuze100_regulator_enable(dev, PMIC_OP_GET, &enable);
	if (ret)
		return ret;

	return enable;
}

static int pfuze100_regulator_set_enable(struct udevice *dev, bool enable)
{
	return pfuze100_regulator_enable(dev, PMIC_OP_SET, &enable);
}

static int pfuze100_regulator_get_mode(struct udevice *dev)
{
	int mode;
	int ret;

	ret = pfuze100_regulator_mode(dev, PMIC_OP_GET, &mode);
	if (ret)
		return ret;

	return mode;
}

static int pfuze100_regulator_set_mode(struct udevice *dev, int mode)
{
	return pfuze100_regulator_mode(dev, PMIC_OP_SET, &mode);
}

static const struct dm_regulator_ops pfuze100_regulator_ops = {
	.get_value  = pfuze100_regulator_get_value,
	.set_value  = pfuze100_regulator_set_value,
	.get_enable = pfuze100_regulator_get_enable,
	.set_enable = pfuze100_regulator_set_enable,
	.get_mode   = pfuze100_regulator_get_mode,
	.set_mode   = pfuze100_regulator_set_mode,
};

U_BOOT_DRIVER(pfuze100_regulator) = {
	.name = "pfuze100_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &pfuze100_regulator_ops,
	.probe = pfuze100_regulator_probe,
	.platdata_auto_alloc_size = sizeof(struct pfuze100_regulator_platdata),
};

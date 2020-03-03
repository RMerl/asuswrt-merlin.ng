/*
 * drivers/regulator/ab3100.c
 *
 * Copyright (C) 2008-2009 ST-Ericsson AB
 * License terms: GNU General Public License (GPL) version 2
 * Low-level control of the AB3100 IC Low Dropout (LDO)
 * regulators, external regulator and buck converter
 * Author: Mattias Wallin <mattias.wallin@stericsson.com>
 * Author: Linus Walleij <linus.walleij@stericsson.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/mfd/ab3100.h>
#include <linux/mfd/abx500.h>
#include <linux/of.h>
#include <linux/regulator/of_regulator.h>

/* LDO registers and some handy masking definitions for AB3100 */
#define AB3100_LDO_A		0x40
#define AB3100_LDO_C		0x41
#define AB3100_LDO_D		0x42
#define AB3100_LDO_E		0x43
#define AB3100_LDO_E_SLEEP	0x44
#define AB3100_LDO_F		0x45
#define AB3100_LDO_G		0x46
#define AB3100_LDO_H		0x47
#define AB3100_LDO_H_SLEEP_MODE	0
#define AB3100_LDO_H_SLEEP_EN	2
#define AB3100_LDO_ON		4
#define AB3100_LDO_H_VSEL_AC	5
#define AB3100_LDO_K		0x48
#define AB3100_LDO_EXT		0x49
#define AB3100_BUCK		0x4A
#define AB3100_BUCK_SLEEP	0x4B
#define AB3100_REG_ON_MASK	0x10

/**
 * struct ab3100_regulator
 * A struct passed around the individual regulator functions
 * @platform_device: platform device holding this regulator
 * @dev: handle to the device
 * @plfdata: AB3100 platform data passed in at probe time
 * @regreg: regulator register number in the AB3100
 */
struct ab3100_regulator {
	struct regulator_dev *rdev;
	struct device *dev;
	struct ab3100_platform_data *plfdata;
	u8 regreg;
};

/* The order in which registers are initialized */
static const u8 ab3100_reg_init_order[AB3100_NUM_REGULATORS+2] = {
	AB3100_LDO_A,
	AB3100_LDO_C,
	AB3100_LDO_E,
	AB3100_LDO_E_SLEEP,
	AB3100_LDO_F,
	AB3100_LDO_G,
	AB3100_LDO_H,
	AB3100_LDO_K,
	AB3100_LDO_EXT,
	AB3100_BUCK,
	AB3100_BUCK_SLEEP,
	AB3100_LDO_D,
};

/* Preset (hardware defined) voltages for these regulators */
#define LDO_A_VOLTAGE 2750000
#define LDO_C_VOLTAGE 2650000
#define LDO_D_VOLTAGE 2650000

static const unsigned int ldo_e_buck_typ_voltages[] = {
	1800000,
	1400000,
	1300000,
	1200000,
	1100000,
	1050000,
	900000,
};

static const unsigned int ldo_f_typ_voltages[] = {
	1800000,
	1400000,
	1300000,
	1200000,
	1100000,
	1050000,
	2500000,
	2650000,
};

static const unsigned int ldo_g_typ_voltages[] = {
	2850000,
	2750000,
	1800000,
	1500000,
};

static const unsigned int ldo_h_typ_voltages[] = {
	2750000,
	1800000,
	1500000,
	1200000,
};

static const unsigned int ldo_k_typ_voltages[] = {
	2750000,
	1800000,
};


/* The regulator devices */
static struct ab3100_regulator
ab3100_regulators[AB3100_NUM_REGULATORS] = {
	{
		.regreg = AB3100_LDO_A,
	},
	{
		.regreg = AB3100_LDO_C,
	},
	{
		.regreg = AB3100_LDO_D,
	},
	{
		.regreg = AB3100_LDO_E,
	},
	{
		.regreg = AB3100_LDO_F,
	},
	{
		.regreg = AB3100_LDO_G,
	},
	{
		.regreg = AB3100_LDO_H,
	},
	{
		.regreg = AB3100_LDO_K,
	},
	{
		.regreg = AB3100_LDO_EXT,
		/* No voltages for the external regulator */
	},
	{
		.regreg = AB3100_BUCK,
	},
};

/*
 * General functions for enable, disable and is_enabled used for
 * LDO: A,C,E,F,G,H,K,EXT and BUCK
 */
static int ab3100_enable_regulator(struct regulator_dev *reg)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	int err;
	u8 regval;

	err = abx500_get_register_interruptible(abreg->dev, 0, abreg->regreg,
						&regval);
	if (err) {
		dev_warn(&reg->dev, "failed to get regid %d value\n",
			 abreg->regreg);
		return err;
	}

	/* The regulator is already on, no reason to go further */
	if (regval & AB3100_REG_ON_MASK)
		return 0;

	regval |= AB3100_REG_ON_MASK;

	err = abx500_set_register_interruptible(abreg->dev, 0, abreg->regreg,
						regval);
	if (err) {
		dev_warn(&reg->dev, "failed to set regid %d value\n",
			 abreg->regreg);
		return err;
	}

	return 0;
}

static int ab3100_disable_regulator(struct regulator_dev *reg)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	int err;
	u8 regval;

	/*
	 * LDO D is a special regulator. When it is disabled, the entire
	 * system is shut down. So this is handled specially.
	 */
	pr_info("Called ab3100_disable_regulator\n");
	if (abreg->regreg == AB3100_LDO_D) {
		dev_info(&reg->dev, "disabling LDO D - shut down system\n");
		/* Setting LDO D to 0x00 cuts the power to the SoC */
		return abx500_set_register_interruptible(abreg->dev, 0,
							 AB3100_LDO_D, 0x00U);
	}

	/*
	 * All other regulators are handled here
	 */
	err = abx500_get_register_interruptible(abreg->dev, 0, abreg->regreg,
						&regval);
	if (err) {
		dev_err(&reg->dev, "unable to get register 0x%x\n",
			abreg->regreg);
		return err;
	}
	regval &= ~AB3100_REG_ON_MASK;
	return abx500_set_register_interruptible(abreg->dev, 0, abreg->regreg,
						 regval);
}

static int ab3100_is_enabled_regulator(struct regulator_dev *reg)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	u8 regval;
	int err;

	err = abx500_get_register_interruptible(abreg->dev, 0, abreg->regreg,
						&regval);
	if (err) {
		dev_err(&reg->dev, "unable to get register 0x%x\n",
			abreg->regreg);
		return err;
	}

	return regval & AB3100_REG_ON_MASK;
}

static int ab3100_get_voltage_regulator(struct regulator_dev *reg)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	u8 regval;
	int err;

	/*
	 * For variable types, read out setting and index into
	 * supplied voltage list.
	 */
	err = abx500_get_register_interruptible(abreg->dev, 0,
						abreg->regreg, &regval);
	if (err) {
		dev_warn(&reg->dev,
			 "failed to get regulator value in register %02x\n",
			 abreg->regreg);
		return err;
	}

	/* The 3 highest bits index voltages */
	regval &= 0xE0;
	regval >>= 5;

	if (regval >= reg->desc->n_voltages) {
		dev_err(&reg->dev,
			"regulator register %02x contains an illegal voltage setting\n",
			abreg->regreg);
		return -EINVAL;
	}

	return reg->desc->volt_table[regval];
}

static int ab3100_set_voltage_regulator_sel(struct regulator_dev *reg,
					    unsigned selector)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	u8 regval;
	int err;

	err = abx500_get_register_interruptible(abreg->dev, 0,
						abreg->regreg, &regval);
	if (err) {
		dev_warn(&reg->dev,
			 "failed to get regulator register %02x\n",
			 abreg->regreg);
		return err;
	}

	/* The highest three bits control the variable regulators */
	regval &= ~0xE0;
	regval |= (selector << 5);

	err = abx500_set_register_interruptible(abreg->dev, 0,
						abreg->regreg, regval);
	if (err)
		dev_warn(&reg->dev, "failed to set regulator register %02x\n",
			abreg->regreg);

	return err;
}

static int ab3100_set_suspend_voltage_regulator(struct regulator_dev *reg,
						int uV)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);
	u8 regval;
	int err;
	int bestindex;
	u8 targetreg;

	if (abreg->regreg == AB3100_LDO_E)
		targetreg = AB3100_LDO_E_SLEEP;
	else if (abreg->regreg == AB3100_BUCK)
		targetreg = AB3100_BUCK_SLEEP;
	else
		return -EINVAL;

	/* LDO E and BUCK have special suspend voltages you can set */
	bestindex = regulator_map_voltage_iterate(reg, uV, uV);

	err = abx500_get_register_interruptible(abreg->dev, 0,
						targetreg, &regval);
	if (err) {
		dev_warn(&reg->dev,
			 "failed to get regulator register %02x\n",
			 targetreg);
		return err;
	}

	/* The highest three bits control the variable regulators */
	regval &= ~0xE0;
	regval |= (bestindex << 5);

	err = abx500_set_register_interruptible(abreg->dev, 0,
						targetreg, regval);
	if (err)
		dev_warn(&reg->dev, "failed to set regulator register %02x\n",
			abreg->regreg);

	return err;
}

/*
 * The external regulator can just define a fixed voltage.
 */
static int ab3100_get_voltage_regulator_external(struct regulator_dev *reg)
{
	struct ab3100_regulator *abreg = rdev_get_drvdata(reg);

	if (abreg->plfdata)
		return abreg->plfdata->external_voltage;
	else
		/* TODO: encode external voltage into device tree */
		return 0;
}

static struct regulator_ops regulator_ops_fixed = {
	.list_voltage = regulator_list_voltage_linear,
	.enable      = ab3100_enable_regulator,
	.disable     = ab3100_disable_regulator,
	.is_enabled  = ab3100_is_enabled_regulator,
};

static struct regulator_ops regulator_ops_variable = {
	.enable      = ab3100_enable_regulator,
	.disable     = ab3100_disable_regulator,
	.is_enabled  = ab3100_is_enabled_regulator,
	.get_voltage = ab3100_get_voltage_regulator,
	.set_voltage_sel = ab3100_set_voltage_regulator_sel,
	.list_voltage = regulator_list_voltage_table,
};

static struct regulator_ops regulator_ops_variable_sleepable = {
	.enable      = ab3100_enable_regulator,
	.disable     = ab3100_disable_regulator,
	.is_enabled  = ab3100_is_enabled_regulator,
	.get_voltage = ab3100_get_voltage_regulator,
	.set_voltage_sel = ab3100_set_voltage_regulator_sel,
	.set_suspend_voltage = ab3100_set_suspend_voltage_regulator,
	.list_voltage = regulator_list_voltage_table,
};

/*
 * LDO EXT is an external regulator so it is really
 * not possible to set any voltage locally here, AB3100
 * is an on/off switch plain an simple. The external
 * voltage is defined in the board set-up if any.
 */
static struct regulator_ops regulator_ops_external = {
	.enable      = ab3100_enable_regulator,
	.disable     = ab3100_disable_regulator,
	.is_enabled  = ab3100_is_enabled_regulator,
	.get_voltage = ab3100_get_voltage_regulator_external,
};

static struct regulator_desc
ab3100_regulator_desc[AB3100_NUM_REGULATORS] = {
	{
		.name = "LDO_A",
		.id   = AB3100_LDO_A,
		.ops  = &regulator_ops_fixed,
		.n_voltages = 1,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.min_uV = LDO_A_VOLTAGE,
		.enable_time = 200,
	},
	{
		.name = "LDO_C",
		.id   = AB3100_LDO_C,
		.ops  = &regulator_ops_fixed,
		.n_voltages = 1,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.min_uV = LDO_C_VOLTAGE,
		.enable_time = 200,
	},
	{
		.name = "LDO_D",
		.id   = AB3100_LDO_D,
		.ops  = &regulator_ops_fixed,
		.n_voltages = 1,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.min_uV = LDO_D_VOLTAGE,
		.enable_time = 200,
	},
	{
		.name = "LDO_E",
		.id   = AB3100_LDO_E,
		.ops  = &regulator_ops_variable_sleepable,
		.n_voltages = ARRAY_SIZE(ldo_e_buck_typ_voltages),
		.volt_table = ldo_e_buck_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 200,
	},
	{
		.name = "LDO_F",
		.id   = AB3100_LDO_F,
		.ops  = &regulator_ops_variable,
		.n_voltages = ARRAY_SIZE(ldo_f_typ_voltages),
		.volt_table = ldo_f_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 600,
	},
	{
		.name = "LDO_G",
		.id   = AB3100_LDO_G,
		.ops  = &regulator_ops_variable,
		.n_voltages = ARRAY_SIZE(ldo_g_typ_voltages),
		.volt_table = ldo_g_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 400,
	},
	{
		.name = "LDO_H",
		.id   = AB3100_LDO_H,
		.ops  = &regulator_ops_variable,
		.n_voltages = ARRAY_SIZE(ldo_h_typ_voltages),
		.volt_table = ldo_h_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 200,
	},
	{
		.name = "LDO_K",
		.id   = AB3100_LDO_K,
		.ops  = &regulator_ops_variable,
		.n_voltages = ARRAY_SIZE(ldo_k_typ_voltages),
		.volt_table = ldo_k_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 200,
	},
	{
		.name = "LDO_EXT",
		.id   = AB3100_LDO_EXT,
		.ops  = &regulator_ops_external,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	{
		.name = "BUCK",
		.id   = AB3100_BUCK,
		.ops  = &regulator_ops_variable_sleepable,
		.n_voltages = ARRAY_SIZE(ldo_e_buck_typ_voltages),
		.volt_table = ldo_e_buck_typ_voltages,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
		.enable_time = 1000,
	},
};

static int ab3100_regulator_register(struct platform_device *pdev,
				     struct ab3100_platform_data *plfdata,
				     struct regulator_init_data *init_data,
				     struct device_node *np,
				     unsigned long id)
{
	struct regulator_desc *desc;
	struct ab3100_regulator *reg;
	struct regulator_dev *rdev;
	struct regulator_config config = { };
	int err, i;

	for (i = 0; i < AB3100_NUM_REGULATORS; i++) {
		desc = &ab3100_regulator_desc[i];
		if (desc->id == id)
			break;
	}
	if (desc->id != id)
		return -ENODEV;

	/* Same index used for this array */
	reg = &ab3100_regulators[i];

	/*
	 * Initialize per-regulator struct.
	 * Inherit platform data, this comes down from the
	 * i2c boarddata, from the machine. So if you want to
	 * see what it looks like for a certain machine, go
	 * into the machine I2C setup.
	 */
	reg->dev = &pdev->dev;
	if (plfdata) {
		reg->plfdata = plfdata;
		config.init_data = &plfdata->reg_constraints[i];
	} else if (np) {
		config.of_node = np;
		config.init_data = init_data;
	}
	config.dev = &pdev->dev;
	config.driver_data = reg;

	rdev = devm_regulator_register(&pdev->dev, desc, &config);
	if (IS_ERR(rdev)) {
		err = PTR_ERR(rdev);
		dev_err(&pdev->dev,
			"%s: failed to register regulator %s err %d\n",
			__func__, desc->name,
			err);
		return err;
	}

	/* Then set a pointer back to the registered regulator */
	reg->rdev = rdev;
	return 0;
}

static struct of_regulator_match ab3100_regulator_matches[] = {
	{ .name = "ab3100_ldo_a", .driver_data = (void *) AB3100_LDO_A, },
	{ .name = "ab3100_ldo_c", .driver_data = (void *) AB3100_LDO_C, },
	{ .name = "ab3100_ldo_d", .driver_data = (void *) AB3100_LDO_D, },
	{ .name = "ab3100_ldo_e", .driver_data = (void *) AB3100_LDO_E, },
	{ .name = "ab3100_ldo_f", .driver_data = (void *) AB3100_LDO_F },
	{ .name = "ab3100_ldo_g", .driver_data = (void *) AB3100_LDO_G },
	{ .name = "ab3100_ldo_h", .driver_data = (void *) AB3100_LDO_H },
	{ .name = "ab3100_ldo_k", .driver_data = (void *) AB3100_LDO_K },
	{ .name = "ab3100_ext", .driver_data = (void *) AB3100_LDO_EXT },
	{ .name = "ab3100_buck", .driver_data = (void *) AB3100_BUCK },
};

/*
 * Initial settings of ab3100 registers.
 * Common for below LDO regulator settings are that
 * bit 7-5 controls voltage. Bit 4 turns regulator ON(1) or OFF(0).
 * Bit 3-2 controls sleep enable and bit 1-0 controls sleep mode.
 */
/* LDO_A 0x16: 2.75V, ON, SLEEP_A, SLEEP OFF GND */
#define LDO_A_SETTING		0x16
/* LDO_C 0x10: 2.65V, ON, SLEEP_A or B, SLEEP full power */
#define LDO_C_SETTING		0x10
/* LDO_D 0x10: 2.65V, ON, sleep mode not used */
#define LDO_D_SETTING		0x10
/* LDO_E 0x10: 1.8V, ON, SLEEP_A or B, SLEEP full power */
#define LDO_E_SETTING		0x10
/* LDO_E SLEEP 0x00: 1.8V, not used, SLEEP_A or B, not used */
#define LDO_E_SLEEP_SETTING	0x00
/* LDO_F 0xD0: 2.5V, ON, SLEEP_A or B, SLEEP full power */
#define LDO_F_SETTING		0xD0
/* LDO_G 0x00: 2.85V, OFF, SLEEP_A or B, SLEEP full power */
#define LDO_G_SETTING		0x00
/* LDO_H 0x18: 2.75V, ON, SLEEP_B, SLEEP full power */
#define LDO_H_SETTING		0x18
/* LDO_K 0x00: 2.75V, OFF, SLEEP_A or B, SLEEP full power */
#define LDO_K_SETTING		0x00
/* LDO_EXT 0x00: Voltage not set, OFF, not used, not used */
#define LDO_EXT_SETTING		0x00
/* BUCK 0x7D: 1.2V, ON, SLEEP_A and B, SLEEP low power */
#define BUCK_SETTING	0x7D
/* BUCK SLEEP 0xAC: 1.05V, Not used, SLEEP_A and B, Not used */
#define BUCK_SLEEP_SETTING	0xAC

static const u8 ab3100_reg_initvals[] = {
	LDO_A_SETTING,
	LDO_C_SETTING,
	LDO_E_SETTING,
	LDO_E_SLEEP_SETTING,
	LDO_F_SETTING,
	LDO_G_SETTING,
	LDO_H_SETTING,
	LDO_K_SETTING,
	LDO_EXT_SETTING,
	BUCK_SETTING,
	BUCK_SLEEP_SETTING,
	LDO_D_SETTING,
};

static int ab3100_regulators_remove(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < AB3100_NUM_REGULATORS; i++) {
		struct ab3100_regulator *reg = &ab3100_regulators[i];

		reg->rdev = NULL;
	}
	return 0;
}

static int
ab3100_regulator_of_probe(struct platform_device *pdev, struct device_node *np)
{
	int err, i;

	/*
	 * Set up the regulator registers, as was previously done with
	 * platform data.
	 */
	/* Set up regulators */
	for (i = 0; i < ARRAY_SIZE(ab3100_reg_init_order); i++) {
		err = abx500_set_register_interruptible(&pdev->dev, 0,
					ab3100_reg_init_order[i],
					ab3100_reg_initvals[i]);
		if (err) {
			dev_err(&pdev->dev, "regulator initialization failed with error %d\n",
				err);
			return err;
		}
	}

	for (i = 0; i < ARRAY_SIZE(ab3100_regulator_matches); i++) {
		err = ab3100_regulator_register(
			pdev, NULL, ab3100_regulator_matches[i].init_data,
			ab3100_regulator_matches[i].of_node,
			(unsigned long)ab3100_regulator_matches[i].driver_data);
		if (err) {
			ab3100_regulators_remove(pdev);
			return err;
		}
	}

	return 0;
}


static int ab3100_regulators_probe(struct platform_device *pdev)
{
	struct ab3100_platform_data *plfdata = dev_get_platdata(&pdev->dev);
	struct device_node *np = pdev->dev.of_node;
	int err = 0;
	u8 data;
	int i;

	/* Check chip state */
	err = abx500_get_register_interruptible(&pdev->dev, 0,
						AB3100_LDO_D, &data);
	if (err) {
		dev_err(&pdev->dev, "could not read initial status of LDO_D\n");
		return err;
	}
	if (data & 0x10)
		dev_notice(&pdev->dev,
			   "chip is already in active mode (Warm start)\n");
	else
		dev_notice(&pdev->dev,
			   "chip is in inactive mode (Cold start)\n");

	if (np) {
		err = of_regulator_match(&pdev->dev, np,
					 ab3100_regulator_matches,
					 ARRAY_SIZE(ab3100_regulator_matches));
		if (err < 0) {
			dev_err(&pdev->dev,
				"Error parsing regulator init data: %d\n", err);
			return err;
		}
		return ab3100_regulator_of_probe(pdev, np);
	}

	/* Set up regulators */
	for (i = 0; i < ARRAY_SIZE(ab3100_reg_init_order); i++) {
		err = abx500_set_register_interruptible(&pdev->dev, 0,
					ab3100_reg_init_order[i],
					plfdata->reg_initvals[i]);
		if (err) {
			dev_err(&pdev->dev, "regulator initialization failed with error %d\n",
				err);
			return err;
		}
	}

	/* Register the regulators */
	for (i = 0; i < AB3100_NUM_REGULATORS; i++) {
		struct regulator_desc *desc = &ab3100_regulator_desc[i];

		err = ab3100_regulator_register(pdev, plfdata, NULL, NULL,
						desc->id);
		if (err) {
			ab3100_regulators_remove(pdev);
			return err;
		}
	}

	return 0;
}

static struct platform_driver ab3100_regulators_driver = {
	.driver = {
		.name  = "ab3100-regulators",
	},
	.probe = ab3100_regulators_probe,
	.remove = ab3100_regulators_remove,
};

static __init int ab3100_regulators_init(void)
{
	return platform_driver_register(&ab3100_regulators_driver);
}

static __exit void ab3100_regulators_exit(void)
{
	platform_driver_unregister(&ab3100_regulators_driver);
}

subsys_initcall(ab3100_regulators_init);
module_exit(ab3100_regulators_exit);

MODULE_AUTHOR("Mattias Wallin <mattias.wallin@stericsson.com>");
MODULE_DESCRIPTION("AB3100 Regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ab3100-regulators");

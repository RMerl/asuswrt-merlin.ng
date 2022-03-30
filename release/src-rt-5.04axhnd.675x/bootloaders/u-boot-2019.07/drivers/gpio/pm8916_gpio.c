// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm pm8916 pmic gpio driver - part of Qualcomm PM8916 PMIC
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/bitops.h>

/* Register offset for each gpio */
#define REG_OFFSET(x)          ((x) * 0x100)

/* Register maps */

/* Type and subtype are shared for all pm8916 peripherals */
#define REG_TYPE               0x4
#define REG_SUBTYPE            0x5

#define REG_STATUS             0x08
#define REG_STATUS_VAL_MASK    0x1

/* MODE_CTL */
#define REG_CTL		0x40
#define REG_CTL_MODE_MASK       0x70
#define REG_CTL_MODE_INPUT      0x00
#define REG_CTL_MODE_INOUT      0x20
#define REG_CTL_MODE_OUTPUT     0x10
#define REG_CTL_OUTPUT_MASK     0x0F

#define REG_DIG_VIN_CTL        0x41
#define REG_DIG_VIN_VIN0       0

#define REG_DIG_PULL_CTL       0x42
#define REG_DIG_PULL_NO_PU     0x5

#define REG_DIG_OUT_CTL        0x45
#define REG_DIG_OUT_CTL_CMOS   (0x0 << 4)
#define REG_DIG_OUT_CTL_DRIVE_L 0x1

#define REG_EN_CTL             0x46
#define REG_EN_CTL_ENABLE      (1 << 7)

struct pm8916_gpio_bank {
	uint32_t pid; /* Peripheral ID on SPMI bus */
};

static int pm8916_gpio_set_direction(struct udevice *dev, unsigned offset,
				     bool input, int value)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	uint32_t gpio_base = priv->pid + REG_OFFSET(offset);
	int ret;

	/* Disable the GPIO */
	ret = pmic_clrsetbits(dev->parent, gpio_base + REG_EN_CTL,
			      REG_EN_CTL_ENABLE, 0);
	if (ret < 0)
		return ret;

	/* Select the mode */
	if (input)
		ret = pmic_reg_write(dev->parent, gpio_base + REG_CTL,
				     REG_CTL_MODE_INPUT);
	else
		ret = pmic_reg_write(dev->parent, gpio_base + REG_CTL,
				     REG_CTL_MODE_INOUT | (value ? 1 : 0));
	if (ret < 0)
		return ret;

	/* Set the right pull (no pull) */
	ret = pmic_reg_write(dev->parent, gpio_base + REG_DIG_PULL_CTL,
			     REG_DIG_PULL_NO_PU);
	if (ret < 0)
		return ret;

	/* Configure output pin drivers if needed */
	if (!input) {
		/* Select the VIN - VIN0, pin is input so it doesn't matter */
		ret = pmic_reg_write(dev->parent, gpio_base + REG_DIG_VIN_CTL,
				     REG_DIG_VIN_VIN0);
		if (ret < 0)
			return ret;

		/* Set the right dig out control */
		ret = pmic_reg_write(dev->parent, gpio_base + REG_DIG_OUT_CTL,
				     REG_DIG_OUT_CTL_CMOS |
				     REG_DIG_OUT_CTL_DRIVE_L);
		if (ret < 0)
			return ret;
	}

	/* Enable the GPIO */
	return pmic_clrsetbits(dev->parent, gpio_base + REG_EN_CTL, 0,
			       REG_EN_CTL_ENABLE);
}

static int pm8916_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	return pm8916_gpio_set_direction(dev, offset, true, 0);
}

static int pm8916_gpio_direction_output(struct udevice *dev, unsigned offset,
					int value)
{
	return pm8916_gpio_set_direction(dev, offset, false, value);
}

static int pm8916_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	uint32_t gpio_base = priv->pid + REG_OFFSET(offset);
	int reg;

	/* Set the output value of the gpio */
	reg = pmic_reg_read(dev->parent, gpio_base + REG_CTL);
	if (reg < 0)
		return reg;

	switch (reg & REG_CTL_MODE_MASK) {
	case REG_CTL_MODE_INPUT:
		return GPIOF_INPUT;
	case REG_CTL_MODE_INOUT: /* Fallthrough */
	case REG_CTL_MODE_OUTPUT:
		return GPIOF_OUTPUT;
	default:
		return GPIOF_UNKNOWN;
	}
}

static int pm8916_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	uint32_t gpio_base = priv->pid + REG_OFFSET(offset);
	int reg;

	reg = pmic_reg_read(dev->parent, gpio_base + REG_STATUS);
	if (reg < 0)
		return reg;

	return !!(reg & REG_STATUS_VAL_MASK);
}

static int pm8916_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	uint32_t gpio_base = priv->pid + REG_OFFSET(offset);

	/* Set the output value of the gpio */
	return pmic_clrsetbits(dev->parent, gpio_base + REG_CTL,
			       REG_CTL_OUTPUT_MASK, !!value);
}

static const struct dm_gpio_ops pm8916_gpio_ops = {
	.direction_input	= pm8916_gpio_direction_input,
	.direction_output	= pm8916_gpio_direction_output,
	.get_value		= pm8916_gpio_get_value,
	.set_value		= pm8916_gpio_set_value,
	.get_function		= pm8916_gpio_get_function,
};

static int pm8916_gpio_probe(struct udevice *dev)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	int reg;

	priv->pid = dev_read_addr(dev);
	if (priv->pid == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Do a sanity check */
	reg = pmic_reg_read(dev->parent, priv->pid + REG_TYPE);
	if (reg != 0x10)
		return -ENODEV;

	reg = pmic_reg_read(dev->parent, priv->pid + REG_SUBTYPE);
	if (reg != 0x5 && reg != 0x1)
		return -ENODEV;

	return 0;
}

static int pm8916_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = dev_read_u32_default(dev, "gpio-count", 0);
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "pm8916";

	return 0;
}

static const struct udevice_id pm8916_gpio_ids[] = {
	{ .compatible = "qcom,pm8916-gpio" },
	{ .compatible = "qcom,pm8994-gpio" },	/* 22 GPIO's */
	{ }
};

U_BOOT_DRIVER(gpio_pm8916) = {
	.name	= "gpio_pm8916",
	.id	= UCLASS_GPIO,
	.of_match = pm8916_gpio_ids,
	.ofdata_to_platdata = pm8916_gpio_ofdata_to_platdata,
	.probe	= pm8916_gpio_probe,
	.ops	= &pm8916_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct pm8916_gpio_bank),
};


/* Add pmic buttons as GPIO as well - there is no generic way for now */
#define PON_INT_RT_STS                        0x10
#define KPDPWR_ON_INT_BIT                     0
#define RESIN_ON_INT_BIT                      1

static int pm8941_pwrkey_get_function(struct udevice *dev, unsigned offset)
{
	return GPIOF_INPUT;
}

static int pm8941_pwrkey_get_value(struct udevice *dev, unsigned offset)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);

	int reg = pmic_reg_read(dev->parent, priv->pid + PON_INT_RT_STS);

	if (reg < 0)
		return 0;

	switch (offset) {
	case 0: /* Power button */
		return (reg & BIT(KPDPWR_ON_INT_BIT)) != 0;
		break;
	case 1: /* Reset button */
	default:
		return (reg & BIT(RESIN_ON_INT_BIT)) != 0;
		break;
	}
}

static const struct dm_gpio_ops pm8941_pwrkey_ops = {
	.get_value		= pm8941_pwrkey_get_value,
	.get_function		= pm8941_pwrkey_get_function,
};

static int pm8941_pwrkey_probe(struct udevice *dev)
{
	struct pm8916_gpio_bank *priv = dev_get_priv(dev);
	int reg;

	priv->pid = devfdt_get_addr(dev);
	if (priv->pid == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Do a sanity check */
	reg = pmic_reg_read(dev->parent, priv->pid + REG_TYPE);
	if (reg != 0x1)
		return -ENODEV;

	reg = pmic_reg_read(dev->parent, priv->pid + REG_SUBTYPE);
	if (reg != 0x1)
		return -ENODEV;

	return 0;
}

static int pm8941_pwrkey_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 2;
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "pm8916_key";

	return 0;
}

static const struct udevice_id pm8941_pwrkey_ids[] = {
	{ .compatible = "qcom,pm8916-pwrkey" },
	{ .compatible = "qcom,pm8994-pwrkey" },
	{ }
};

U_BOOT_DRIVER(pwrkey_pm8941) = {
	.name	= "pwrkey_pm8916",
	.id	= UCLASS_GPIO,
	.of_match = pm8941_pwrkey_ids,
	.ofdata_to_platdata = pm8941_pwrkey_ofdata_to_platdata,
	.probe	= pm8941_pwrkey_probe,
	.ops	= &pm8941_pwrkey_ops,
	.priv_auto_alloc_size = sizeof(struct pm8916_gpio_bank),
};

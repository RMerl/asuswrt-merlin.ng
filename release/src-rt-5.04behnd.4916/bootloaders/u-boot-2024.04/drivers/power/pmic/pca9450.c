// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <i2c.h>
#include <linux/err.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm-generic/gpio.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/pca9450.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
	/* buck */
	{ .prefix = "b", .driver = PCA9450_REGULATOR_DRIVER},
	{ .prefix = "B", .driver = PCA9450_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = PCA9450_REGULATOR_DRIVER},
	{ .prefix = "L", .driver = PCA9450_REGULATOR_DRIVER},
	{ },
};

struct pca9450_priv {
	struct gpio_desc *sd_vsel_gpio;
};

static int pca9450_reg_count(struct udevice *dev)
{
	return PCA9450_REG_NUM;
}

static int pca9450_write(struct udevice *dev, uint reg, const uint8_t *buff,
			 int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_read(struct udevice *dev, uint reg, uint8_t *buff,
			int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pca9450_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node,
				      pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static int pca9450_probe(struct udevice *dev)
{
	struct pca9450_priv *priv = dev_get_priv(dev);
	unsigned int reset_ctrl;
	int ret = 0;

	if (CONFIG_IS_ENABLED(DM_GPIO) && CONFIG_IS_ENABLED(DM_REGULATOR_PCA9450)) {
		priv->sd_vsel_gpio = devm_gpiod_get_optional(dev, "sd-vsel",
							     GPIOD_IS_OUT |
							     GPIOD_IS_OUT_ACTIVE);
		if (IS_ERR(priv->sd_vsel_gpio)) {
			ret = PTR_ERR(priv->sd_vsel_gpio);
			dev_err(dev, "Failed to request SD_VSEL GPIO: %d\n", ret);
			if (ret)
				return ret;
		}
	}

	if (ofnode_read_bool(dev_ofnode(dev), "nxp,wdog_b-warm-reset"))
		reset_ctrl = PCA9450_PMIC_RESET_WDOG_B_CFG_WARM;
	else
		reset_ctrl = PCA9450_PMIC_RESET_WDOG_B_CFG_COLD_LDO12;

	return pmic_clrsetbits(dev, PCA9450_RESET_CTRL,
			       PCA9450_PMIC_RESET_WDOG_B_CFG_MASK, reset_ctrl);
}

static struct dm_pmic_ops pca9450_ops = {
	.reg_count = pca9450_reg_count,
	.read = pca9450_read,
	.write = pca9450_write,
};

static const struct udevice_id pca9450_ids[] = {
	{ .compatible = "nxp,pca9450a", .data = NXP_CHIP_TYPE_PCA9450A, },
	{ .compatible = "nxp,pca9450b", .data = NXP_CHIP_TYPE_PCA9450BC, },
	{ .compatible = "nxp,pca9450c", .data = NXP_CHIP_TYPE_PCA9450BC, },
	{ .compatible = "nxp,pca9451a", .data = NXP_CHIP_TYPE_PCA9451A, },
	{ }
};

U_BOOT_DRIVER(pmic_pca9450) = {
	.name = "pca9450 pmic",
	.id = UCLASS_PMIC,
	.of_match = pca9450_ids,
	.bind = pca9450_bind,
	.probe = pca9450_probe,
	.ops = &pca9450_ops,
	.priv_auto = sizeof(struct pca9450_priv),
};

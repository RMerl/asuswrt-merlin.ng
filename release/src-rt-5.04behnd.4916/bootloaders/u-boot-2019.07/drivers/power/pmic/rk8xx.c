// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power/rk8xx_pmic.h>
#include <power/pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "DCDC_REG", .driver = "rk8xx_buck"},
	{ .prefix = "LDO_REG", .driver = "rk8xx_ldo"},
	{ .prefix = "SWITCH_REG", .driver = "rk8xx_switch"},
	{ },
};

static int rk8xx_reg_count(struct udevice *dev)
{
	return RK808_NUM_OF_REGS;
}

static int rk8xx_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret) {
		debug("write error to device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int rk8xx_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		debug("read error from device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
static int rk8xx_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}
#endif

static int rk8xx_probe(struct udevice *dev)
{
	struct rk8xx_priv *priv = dev_get_priv(dev);
	uint8_t msb, lsb;

	/* read Chip variant */
	rk8xx_read(dev, ID_MSB, &msb, 1);
	rk8xx_read(dev, ID_LSB, &lsb, 1);

	priv->variant = ((msb << 8) | lsb) & RK8XX_ID_MSK;

	return 0;
}

static struct dm_pmic_ops rk8xx_ops = {
	.reg_count = rk8xx_reg_count,
	.read = rk8xx_read,
	.write = rk8xx_write,
};

static const struct udevice_id rk8xx_ids[] = {
	{ .compatible = "rockchip,rk808" },
	{ .compatible = "rockchip,rk818" },
	{ }
};

U_BOOT_DRIVER(pmic_rk8xx) = {
	.name = "rk8xx pmic",
	.id = UCLASS_PMIC,
	.of_match = rk8xx_ids,
#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
	.bind = rk8xx_bind,
#endif
	.priv_auto_alloc_size   = sizeof(struct rk8xx_priv),
	.probe = rk8xx_probe,
	.ops = &rk8xx_ops,
};

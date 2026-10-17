// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Texas Instruments Incorporated, <www.ti.com>
 * Keerthy <j-keerthy@ti.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/lists.h>
#include <i2c.h>
#include <log.h>
#include <linux/printk.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/palmas.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = PALMAS_LDO_DRIVER },
	{ .prefix = "smps", .driver = PALMAS_SMPS_DRIVER },
	{ },
};

static int palmas_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int palmas_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int palmas_bind(struct udevice *dev)
{
	ofnode pmic_node = ofnode_null(), regulators_node;
	ofnode subnode, gpio_node;
	int children, ret;

	if (IS_ENABLED(CONFIG_SYSRESET_PALMAS)) {
		ret = device_bind_driver(dev, PALMAS_RST_DRIVER,
					 "sysreset", NULL);
		if (ret) {
			log_err("cannot bind SYSRESET (ret = %d)\n", ret);
			return ret;
		}
	}

	gpio_node = ofnode_find_subnode(dev_ofnode(dev), "gpio");
	if (ofnode_valid(gpio_node)) {
		ret = device_bind_driver_to_node(dev, PALMAS_GPIO_DRIVER,
						 "gpio", gpio_node, NULL);
		if (ret)
			log_err("cannot bind GPIOs (ret = %d)\n", ret);
	}

	dev_for_each_subnode(subnode, dev) {
		const char *name;
		char *temp;

		name = ofnode_get_name(subnode);
		temp = strstr(name, "pmic");
		if (temp) {
			pmic_node = subnode;
			break;
		}
	}

	if (!ofnode_valid(pmic_node)) {
		debug("%s: %s pmic subnode not found!\n", __func__, dev->name);
		return -ENXIO;
	}

	regulators_node = ofnode_find_subnode(pmic_node, "regulators");

	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s reg subnode not found!\n", __func__, dev->name);
		return -ENXIO;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}

static int palmas_probe(struct udevice *dev)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct palmas_priv *priv = dev_get_priv(dev);
	struct udevice *bus = dev_get_parent(dev);
	u32 chip2_addr = chip->chip_addr + 1;
	int ret;

	/* Palmas PMIC is multi chip and chips are located in a row */
	ret = i2c_get_chip(bus, chip2_addr, 1, &priv->chip2);
	if (ret) {
		log_err("cannot get second PMIC I2C chip (err %d)\n", ret);
		return ret;
	}

	return 0;
}

static struct dm_pmic_ops palmas_ops = {
	.read = palmas_read,
	.write = palmas_write,
};

static const struct udevice_id palmas_ids[] = {
	{ .compatible = "ti,tps659038", .data = TPS659038 },
	{ .compatible = "ti,tps65913" , .data = TPS659038 },
	{ .compatible = "ti,tps65917" , .data = TPS65917 },
	{ }
};

U_BOOT_DRIVER(pmic_palmas) = {
	.name = "palmas_pmic",
	.id = UCLASS_PMIC,
	.of_match = palmas_ids,
	.bind = palmas_bind,
	.probe = palmas_probe,
	.ops = &palmas_ops,
	.priv_auto = sizeof(struct palmas_priv),
};

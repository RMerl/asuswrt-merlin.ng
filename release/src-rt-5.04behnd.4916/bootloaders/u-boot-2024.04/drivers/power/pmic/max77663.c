// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <power/pmic.h>
#include <power/max77663.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = MAX77663_LDO_DRIVER },
	{ .prefix = "sd", .driver = MAX77663_SD_DRIVER },
	{ },
};

static int max77663_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret) {
		log_debug("write error to device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int max77663_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		log_debug("read error from device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int max77663_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children, ret;

	if (IS_ENABLED(CONFIG_SYSRESET_MAX77663)) {
		ret = device_bind_driver(dev, MAX77663_RST_DRIVER,
					 "sysreset", NULL);
		if (ret) {
			log_err("cannot bind SYSRESET (ret = %d)\n", ret);
			return ret;
		}
	}

	if (IS_ENABLED(CONFIG_MAX77663_GPIO)) {
		ret = device_bind_driver(dev, MAX77663_GPIO_DRIVER,
					 "gpio", NULL);
		if (ret) {
			log_err("cannot bind GPIOs (ret = %d)\n", ret);
			return ret;
		}
	}

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		log_err("%s regulators subnode not found!\n", dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		log_err("%s - no child found\n", dev->name);

	/* Always return success for this device */
	return 0;
}

static struct dm_pmic_ops max77663_ops = {
	.read = max77663_read,
	.write = max77663_write,
};

static const struct udevice_id max77663_ids[] = {
	{ .compatible = "maxim,max77663" },
	{ }
};

U_BOOT_DRIVER(pmic_max77663) = {
	.name = "max77663_pmic",
	.id = UCLASS_PMIC,
	.of_match = max77663_ids,
	.bind = max77663_bind,
	.ops = &max77663_ops,
};

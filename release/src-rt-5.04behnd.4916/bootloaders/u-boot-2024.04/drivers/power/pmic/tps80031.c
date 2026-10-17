// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <power/pmic.h>
#include <power/tps80031.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "ldo", .driver = TPS80031_LDO_DRIVER },
	{ .prefix = "smps", .driver = TPS80031_SMPS_DRIVER },
	{ },
};

static int tps80031_write(struct udevice *dev, uint reg, const uint8_t *buff,
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

static int tps80031_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		log_debug("read error from device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int tps80031_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children, ret;

	if (IS_ENABLED(CONFIG_SYSRESET_TPS80031)) {
		ret = device_bind_driver(dev, TPS80031_RST_DRIVER,
					 "sysreset", NULL);
		if (ret) {
			log_err("cannot bind SYSRESET (ret = %d)\n", ret);
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

static struct dm_pmic_ops tps80031_ops = {
	.read = tps80031_read,
	.write = tps80031_write,
};

static const struct udevice_id tps80031_ids[] = {
	{ .compatible = "ti,tps80031" },
	{ .compatible = "ti,tps80032" },
	{ }
};

U_BOOT_DRIVER(pmic_tps80031) = {
	.name = "tps80031_pmic",
	.id = UCLASS_PMIC,
	.of_match = tps80031_ids,
	.bind = tps80031_bind,
	.ops = &tps80031_ops,
};

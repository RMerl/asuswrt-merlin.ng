// SPDX-License-Identifier: GPL-2.0+

#include <axp_pmic.h>
#include <dm.h>
#include <dm/lists.h>
#include <i2c.h>
#include <power/pmic.h>
#include <sysreset.h>

#if CONFIG_IS_ENABLED(SYSRESET)
static int axp_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int ret;

	if (type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	ret = pmic_clrsetbits(dev->parent, AXP152_SHUTDOWN, 0, AXP152_POWEROFF);
	if (ret < 0)
		return ret;

	return -EINPROGRESS;
}

static struct sysreset_ops axp_sysreset_ops = {
	.request	= axp_sysreset_request,
};

U_BOOT_DRIVER(axp_sysreset) = {
	.name		= "axp_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &axp_sysreset_ops,
};
#endif

static int axp_pmic_reg_count(struct udevice *dev)
{
	/* TODO: Get the specific value from driver data. */
	return 0x100;
}

static struct dm_pmic_ops axp_pmic_ops = {
	.reg_count	= axp_pmic_reg_count,
	.read		= dm_i2c_read,
	.write		= dm_i2c_write,
};

static const struct pmic_child_info axp_pmic_child_info[] = {
	{ "aldo",	"axp_regulator" },
	{ "bldo",	"axp_regulator" },
	{ "cldo",	"axp_regulator" },
	{ "dc",		"axp_regulator" },
	{ "dldo",	"axp_regulator" },
	{ "eldo",	"axp_regulator" },
	{ "fldo",	"axp_regulator" },
	{ "ldo",	"axp_regulator" },
	{ "sw",		"axp_regulator" },
	{ }
};

static int axp_pmic_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int ret;

	ret = dm_scan_fdt_dev(dev);
	if (ret)
		return ret;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (ofnode_valid(regulators_node))
		pmic_bind_children(dev, regulators_node, axp_pmic_child_info);

	if (CONFIG_IS_ENABLED(SYSRESET)) {
		ret = device_bind_driver_to_node(dev, "axp_sysreset", "axp_sysreset",
						 dev_ofnode(dev), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id axp_pmic_ids[] = {
	{ .compatible = "x-powers,axp152", .data = AXP152_ID },
	{ .compatible = "x-powers,axp202", .data = AXP202_ID },
	{ .compatible = "x-powers,axp209", .data = AXP209_ID },
	{ .compatible = "x-powers,axp221", .data = AXP221_ID },
	{ .compatible = "x-powers,axp223", .data = AXP223_ID },
	{ .compatible = "x-powers,axp313a", .data = AXP313_ID },
	{ .compatible = "x-powers,axp803", .data = AXP803_ID },
	{ .compatible = "x-powers,axp806", .data = AXP806_ID },
	{ .compatible = "x-powers,axp809", .data = AXP809_ID },
	{ .compatible = "x-powers,axp813", .data = AXP813_ID },
	{ }
};

U_BOOT_DRIVER(axp_pmic) = {
	.name		= "axp_pmic",
	.id		= UCLASS_PMIC,
	.of_match	= axp_pmic_ids,
	.bind		= axp_pmic_bind,
	.ops		= &axp_pmic_ops,
};

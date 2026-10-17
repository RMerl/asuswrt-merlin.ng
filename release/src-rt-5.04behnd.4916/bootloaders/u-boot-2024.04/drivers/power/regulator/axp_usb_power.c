// SPDX-License-Identifier: GPL-2.0+

#include <dm/device.h>
#include <errno.h>
#include <power/pmic.h>
#include <power/regulator.h>

#define AXP_POWER_STATUS		0x00
#define AXP_POWER_STATUS_VBUS_PRESENT		BIT(5)

static int axp_usb_power_get_enable(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, AXP_POWER_STATUS);
	if (ret < 0)
		return ret;

	return !!(ret & AXP_POWER_STATUS_VBUS_PRESENT);
}

static const struct dm_regulator_ops axp_usb_power_ops = {
	.get_enable		= axp_usb_power_get_enable,
};

static int axp_usb_power_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->type = REGULATOR_TYPE_FIXED;

	return 0;
}

static const struct udevice_id axp_usb_power_ids[] = {
	{ .compatible = "x-powers,axp202-usb-power-supply" },
	{ .compatible = "x-powers,axp221-usb-power-supply" },
	{ .compatible = "x-powers,axp223-usb-power-supply" },
	{ .compatible = "x-powers,axp813-usb-power-supply" },
	{ }
};

U_BOOT_DRIVER(axp_usb_power) = {
	.name		= "axp_usb_power",
	.id		= UCLASS_REGULATOR,
	.of_match	= axp_usb_power_ids,
	.probe		= axp_usb_power_probe,
	.ops		= &axp_usb_power_ops,
};

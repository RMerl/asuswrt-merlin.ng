// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm pm8916 pmic driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */
#include <common.h>
#include <dm.h>
#include <power/pmic.h>
#include <spmi/spmi.h>

#define PID_SHIFT 8
#define PID_MASK (0xFF << PID_SHIFT)
#define REG_MASK 0xFF

struct pm8916_priv {
	uint32_t usid; /* Slave ID on SPMI bus */
};

static int pm8916_reg_count(struct udevice *dev)
{
	return 0xFFFF;
}

static int pm8916_write(struct udevice *dev, uint reg, const uint8_t *buff,
			int len)
{
	struct pm8916_priv *priv = dev_get_priv(dev);

	if (len != 1)
		return -EINVAL;

	return spmi_reg_write(dev->parent, priv->usid,
			      (reg & PID_MASK) >> PID_SHIFT, reg & REG_MASK,
			      *buff);
}

static int pm8916_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	struct pm8916_priv *priv = dev_get_priv(dev);
	int val;

	if (len != 1)
		return -EINVAL;

	val = spmi_reg_read(dev->parent, priv->usid,
			    (reg & PID_MASK) >> PID_SHIFT, reg & REG_MASK);

	if (val < 0)
		return val;
	*buff = val;
	return 0;
}

static struct dm_pmic_ops pm8916_ops = {
	.reg_count = pm8916_reg_count,
	.read = pm8916_read,
	.write = pm8916_write,
};

static const struct udevice_id pm8916_ids[] = {
	{ .compatible = "qcom,spmi-pmic" },
	{ }
};

static int pm8916_probe(struct udevice *dev)
{
	struct pm8916_priv *priv = dev_get_priv(dev);

	priv->usid = dev_read_addr(dev);

	if (priv->usid == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(pmic_pm8916) = {
	.name = "pmic_pm8916",
	.id = UCLASS_PMIC,
	.of_match = pm8916_ids,
	.bind = dm_scan_fdt_dev,
	.probe = pm8916_probe,
	.ops = &pm8916_ops,
	.priv_auto_alloc_size = sizeof(struct pm8916_priv),
};

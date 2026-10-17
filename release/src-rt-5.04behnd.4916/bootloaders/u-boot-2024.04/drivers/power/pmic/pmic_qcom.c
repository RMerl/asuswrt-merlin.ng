// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm generic pmic driver
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

struct pmic_qcom_priv {
	uint32_t usid; /* Slave ID on SPMI bus */
};

static int pmic_qcom_reg_count(struct udevice *dev)
{
	return 0xFFFF;
}

static int pmic_qcom_write(struct udevice *dev, uint reg, const uint8_t *buff,
			   int len)
{
	struct pmic_qcom_priv *priv = dev_get_priv(dev);

	if (len != 1)
		return -EINVAL;

	return spmi_reg_write(dev->parent, priv->usid,
			      (reg & PID_MASK) >> PID_SHIFT, reg & REG_MASK,
			      *buff);
}

static int pmic_qcom_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	struct pmic_qcom_priv *priv = dev_get_priv(dev);
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

static struct dm_pmic_ops pmic_qcom_ops = {
	.reg_count = pmic_qcom_reg_count,
	.read = pmic_qcom_read,
	.write = pmic_qcom_write,
};

static const struct udevice_id pmic_qcom_ids[] = {
	{ .compatible = "qcom,spmi-pmic" },
	{ }
};

static int pmic_qcom_probe(struct udevice *dev)
{
	struct pmic_qcom_priv *priv = dev_get_priv(dev);
	int ret;

	/*
	 * dev_read_addr() can't be used here because the reg property actually
	 * contains two discrete values, not a single 64-bit address.
	 * The address is the first value.
	 */
	ret = ofnode_read_u32_index(dev_ofnode(dev), "reg", 0, &priv->usid);
	if (ret < 0)
		return -EINVAL;

	debug("usid: %d\n", priv->usid);

	return 0;
}

U_BOOT_DRIVER(pmic_qcom) = {
	.name = "pmic_qcom",
	.id = UCLASS_PMIC,
	.of_match = pmic_qcom_ids,
	.bind = dm_scan_fdt_dev,
	.probe = pmic_qcom_probe,
	.ops = &pmic_qcom_ops,
	.priv_auto	= sizeof(struct pmic_qcom_priv),
};

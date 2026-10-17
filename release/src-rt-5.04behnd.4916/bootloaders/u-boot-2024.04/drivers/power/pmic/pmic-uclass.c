// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#define LOG_CATEGORY UCLASS_PMIC

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <vsprintf.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <linux/ctype.h>

#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
int pmic_bind_children(struct udevice *pmic, ofnode parent,
		       const struct pmic_child_info *child_info)
{
	const struct pmic_child_info *info;
	struct driver *drv;
	struct udevice *child;
	const char *node_name;
	const char *reg_name;
	int bind_count = 0;
	ofnode node;
	int prefix_len;
	int ret;

	debug("%s for '%s' at node offset: %d\n", __func__, pmic->name,
	      dev_of_offset(pmic));

	ofnode_for_each_subnode(node, parent) {
		node_name = ofnode_get_name(node);

		debug("* Found child node: '%s'\n", node_name);
		if (!ofnode_is_enabled(node)) {
			debug("  - ignoring disabled device\n");
			continue;
		}

		child = NULL;
		for (info = child_info; info->prefix && info->driver; info++) {
			debug("  - compatible prefix: '%s'\n", info->prefix);

			prefix_len = strlen(info->prefix);
			if (strncmp(info->prefix, node_name, prefix_len)) {
				reg_name = ofnode_read_string(node,
							      "regulator-name");
				if (!reg_name)
					continue;
				if (strncmp(info->prefix, reg_name, prefix_len))
					continue;
			}

			drv = lists_driver_lookup_name(info->driver);
			if (!drv) {
				debug("  - driver: '%s' not found!\n",
				      info->driver);
				continue;
			}

			debug("  - found child driver: '%s'\n", drv->name);

			ret = device_bind_with_driver_data(pmic, drv, node_name,
							   0, node, &child);
			if (ret) {
				debug("  - child binding error: %d\n", ret);
				continue;
			}

			debug("  - bound child device: '%s'\n", child->name);

			child->driver_data = trailing_strtol(node_name);

			debug("  - set 'child->driver_data': %lu\n",
			      child->driver_data);
			break;
		}

		if (child)
			bind_count++;
		else
			debug("  - compatible prefix not found\n");
	}

	debug("Bound: %d children for PMIC: '%s'\n", bind_count, pmic->name);
	return bind_count;
}
#endif

int pmic_get(const char *name, struct udevice **devp)
{
	return uclass_get_device_by_name(UCLASS_PMIC, name, devp);
}

int pmic_reg_count(struct udevice *dev)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->reg_count)
		return -ENOSYS;

	return ops->reg_count(dev);
}

int pmic_read(struct udevice *dev, uint reg, uint8_t *buffer, int len)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!buffer)
		return -EFAULT;

	if (!ops || !ops->read)
		return -ENOSYS;

	return ops->read(dev, reg, buffer, len);
}

int pmic_write(struct udevice *dev, uint reg, const uint8_t *buffer, int len)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!buffer)
		return -EFAULT;

	if (!ops || !ops->write)
		return -ENOSYS;

	return ops->write(dev, reg, buffer, len);
}

int pmic_reg_read(struct udevice *dev, uint reg)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);
	u32 val = 0;
	int ret;

	if (priv->trans_len < 1 || priv->trans_len > sizeof(val)) {
		debug("Wrong transmission size [%d]\n", priv->trans_len);
		return -EINVAL;
	}

	debug("%s: reg=%x priv->trans_len:%d", __func__, reg, priv->trans_len);
	ret = pmic_read(dev, reg, (uint8_t *)&val, priv->trans_len);
	debug(", value=%x, ret=%d\n", val, ret);

	return ret ? ret : val;
}

int pmic_reg_write(struct udevice *dev, uint reg, uint value)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	if (priv->trans_len < 1 || priv->trans_len > sizeof(value)) {
		debug("Wrong transmission size [%d]\n", priv->trans_len);
		return -EINVAL;
	}

	debug("%s: reg=%x, value=%x priv->trans_len:%d", __func__, reg, value,
	      priv->trans_len);
	ret = pmic_write(dev, reg, (uint8_t *)&value, priv->trans_len);
	debug(", ret=%d\n", ret);

	return ret;
}

int pmic_clrsetbits(struct udevice *dev, uint reg, uint clr, uint set)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);
	u32 val = 0;
	int ret;

	if (priv->trans_len < 1 || priv->trans_len > sizeof(val)) {
		debug("Wrong transmission size [%d]\n", priv->trans_len);
		return -EINVAL;
	}

	ret = pmic_read(dev, reg, (uint8_t *)&val, priv->trans_len);
	if (ret < 0)
		return ret;

	val = (val & ~clr) | set;
	return pmic_write(dev, reg, (uint8_t *)&val, priv->trans_len);
}

static int pmic_pre_probe(struct udevice *dev)
{
	struct uc_pmic_priv *pmic_priv = dev_get_uclass_priv(dev);

	pmic_priv->trans_len = 1;
	return 0;
}

UCLASS_DRIVER(pmic) = {
	.id		= UCLASS_PMIC,
	.name		= "pmic",
	.pre_probe	= pmic_pre_probe,
	.per_device_auto	= sizeof(struct uc_pmic_priv),
};

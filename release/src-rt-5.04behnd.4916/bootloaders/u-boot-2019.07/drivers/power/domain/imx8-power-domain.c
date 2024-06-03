// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 NXP
 */

#include <common.h>
#include <dm.h>
#include <power-domain-uclass.h>
#include <asm/io.h>
#include <asm/arch/power-domain.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <asm/arch/sci/sci.h>

DECLARE_GLOBAL_DATA_PTR;

struct imx8_power_domain_priv {
	bool state_on;
};

static int imx8_power_domain_request(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return 0;
}

static int imx8_power_domain_free(struct power_domain *power_domain)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	return 0;
}

static int imx8_power_domain_on(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8_power_domain_platdata *pdata;
	struct imx8_power_domain_priv *ppriv;
	sc_err_t ret;
	int err;

	struct power_domain parent_domain;
	struct udevice *parent = dev_get_parent(dev);

	/* Need to power on parent node first */
	if (device_get_uclass_id(parent) == UCLASS_POWER_DOMAIN) {
		parent_domain.dev = parent;
		err = imx8_power_domain_on(&parent_domain);
		if (err)
			return err;
	}

	pdata = (struct imx8_power_domain_platdata *)dev_get_platdata(dev);
	ppriv = (struct imx8_power_domain_priv *)dev_get_priv(dev);

	debug("%s(power_domain=%s) resource_id %d\n", __func__, dev->name,
	      pdata->resource_id);

	/* Already powered on */
	if (ppriv->state_on)
		return 0;

	if (pdata->resource_id != SC_R_LAST) {
		ret = sc_pm_set_resource_power_mode(-1, pdata->resource_id,
						    SC_PM_PW_MODE_ON);
		if (ret) {
			printf("Error: %s Power up failed! (error = %d)\n",
			       dev->name, ret);
			return -EIO;
		}
	}

	ppriv->state_on = true;
	debug("%s is powered on\n", dev->name);

	return 0;
}

static int imx8_power_domain_off_node(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct udevice *child;
	struct imx8_power_domain_priv *ppriv;
	struct imx8_power_domain_priv *child_ppriv;
	struct imx8_power_domain_platdata *pdata;
	sc_err_t ret;

	ppriv = dev_get_priv(dev);
	pdata = dev_get_platdata(dev);

	debug("%s, %s, state_on %d\n", __func__, dev->name, ppriv->state_on);

	/* Already powered off */
	if (!ppriv->state_on)
		return 0;

	/* Check if all subnodes are off */
	for (device_find_first_child(dev, &child);
		child;
		device_find_next_child(&child)) {
		if (device_active(child)) {
			child_ppriv =
			(struct imx8_power_domain_priv *)dev_get_priv(child);
			if (child_ppriv->state_on)
				return -EPERM;
		}
	}

	if (pdata->resource_id != SC_R_LAST) {
		if (!sc_rm_is_resource_owned(-1, pdata->resource_id)) {
			printf("%s not owned by curr partition\n", dev->name);
			return 0;
		}
		ret = sc_pm_set_resource_power_mode(-1, pdata->resource_id,
						    SC_PM_PW_MODE_OFF);
		if (ret) {
			printf("Error: %s Power off failed! (error = %d)\n",
			       dev->name, ret);
			return -EIO;
		}
	}

	ppriv->state_on = false;
	debug("%s is powered off\n", dev->name);

	return 0;
}

static int imx8_power_domain_off_parentnodes(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct udevice *parent = dev_get_parent(dev);
	struct udevice *child;
	struct imx8_power_domain_priv *ppriv;
	struct imx8_power_domain_priv *child_ppriv;
	struct imx8_power_domain_platdata *pdata;
	sc_err_t ret;
	struct power_domain parent_pd;

	if (device_get_uclass_id(parent) == UCLASS_POWER_DOMAIN) {
		pdata =
		(struct imx8_power_domain_platdata *)dev_get_platdata(parent);
		ppriv = (struct imx8_power_domain_priv *)dev_get_priv(parent);

		debug("%s, %s, state_on %d\n", __func__, parent->name,
		      ppriv->state_on);

		/* Already powered off */
		if (!ppriv->state_on)
			return 0;

		/*
		 * Check if all sibling nodes are off. If yes,
		 * power off parent
		 */
		for (device_find_first_child(parent, &child); child;
		     device_find_next_child(&child)) {
			if (device_active(child)) {
				child_ppriv = (struct imx8_power_domain_priv *)
						dev_get_priv(child);
				/* Find a power on sibling */
				if (child_ppriv->state_on) {
					debug("sibling %s, state_on %d\n",
					      child->name,
					      child_ppriv->state_on);
					return 0;
				}
			}
		}

		/* power off parent */
		if (pdata->resource_id != SC_R_LAST) {
			ret = sc_pm_set_resource_power_mode(-1,
							    pdata->resource_id,
							    SC_PM_PW_MODE_OFF);
			if (ret) {
				printf("%s Power off failed! (error = %d)\n",
				       parent->name, ret);
				return -EIO;
			}
		}

		ppriv->state_on = false;
		debug("%s is powered off\n", parent->name);

		parent_pd.dev = parent;
		imx8_power_domain_off_parentnodes(&parent_pd);
	}

	return 0;
}

static int imx8_power_domain_off(struct power_domain *power_domain)
{
	int ret;

	debug("%s(power_domain=%p)\n", __func__, power_domain);

	/* Turn off the node */
	ret = imx8_power_domain_off_node(power_domain);
	if (ret) {
		debug("Can't power off the node of dev %s, ret = %d\n",
		      power_domain->dev->name, ret);
		return ret;
	}

	/* Turn off parent nodes, if sibling nodes are all off */
	ret = imx8_power_domain_off_parentnodes(power_domain);
	if (ret) {
		printf("Failed to power off parent nodes of dev %s, ret = %d\n",
		       power_domain->dev->name, ret);
		return ret;
	}

	return 0;
}

static int imx8_power_domain_of_xlate(struct power_domain *power_domain,
				      struct ofnode_phandle_args *args)
{
	debug("%s(power_domain=%p)\n", __func__, power_domain);

	/* Do nothing to the xlate, since we don't have args used */

	return 0;
}

static int imx8_power_domain_bind(struct udevice *dev)
{
	int offset;
	const char *name;
	int ret = 0;

	debug("%s(dev=%p)\n", __func__, dev);

	offset = dev_of_offset(dev);
	for (offset = fdt_first_subnode(gd->fdt_blob, offset); offset > 0;
	     offset = fdt_next_subnode(gd->fdt_blob, offset)) {
		/* Bind the subnode to this driver */
		name = fdt_get_name(gd->fdt_blob, offset, NULL);

		ret = device_bind_with_driver_data(dev, dev->driver, name,
						   dev->driver_data,
						   offset_to_ofnode(offset),
						   NULL);

		if (ret == -ENODEV)
			printf("Driver '%s' refuses to bind\n",
			       dev->driver->name);

		if (ret)
			printf("Error binding driver '%s': %d\n",
			       dev->driver->name, ret);
	}

	return 0;
}

static int imx8_power_domain_probe(struct udevice *dev)
{
	struct imx8_power_domain_priv *ppriv;

	debug("%s(dev=%s)\n", __func__, dev->name);

	ppriv = (struct imx8_power_domain_priv *)dev_get_priv(dev);

	/* Set default to power off */
	if (ppriv)
		ppriv->state_on = false;

	return 0;
}

static int imx8_power_domain_ofdata_to_platdata(struct udevice *dev)
{
	int reg;
	struct imx8_power_domain_platdata *pdata = dev_get_platdata(dev);

	reg = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "reg", -1);
	if (reg == -1) {
		debug("%s: Invalid resource id %d\n", __func__, reg);
		return -EINVAL;
	}
	pdata->resource_id = (sc_rsrc_t)reg;

	debug("%s resource_id %d\n", __func__, pdata->resource_id);

	return 0;
}

static const struct udevice_id imx8_power_domain_ids[] = {
	{ .compatible = "nxp,imx8-pd" },
	{ }
};

struct power_domain_ops imx8_power_domain_ops = {
	.request = imx8_power_domain_request,
	.free = imx8_power_domain_free,
	.on = imx8_power_domain_on,
	.off = imx8_power_domain_off,
	.of_xlate = imx8_power_domain_of_xlate,
};

U_BOOT_DRIVER(imx8_power_domain) = {
	.name = "imx8_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = imx8_power_domain_ids,
	.bind = imx8_power_domain_bind,
	.probe = imx8_power_domain_probe,
	.ofdata_to_platdata = imx8_power_domain_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct imx8_power_domain_platdata),
	.priv_auto_alloc_size = sizeof(struct imx8_power_domain_priv),
	.ops = &imx8_power_domain_ops,
};

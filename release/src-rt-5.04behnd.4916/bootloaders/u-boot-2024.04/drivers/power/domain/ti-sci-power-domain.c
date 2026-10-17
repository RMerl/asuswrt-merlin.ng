// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface (TI SCI) power domain driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 *
 * Loosely based on Linux kernel ti_sci_pm_domains.c...
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <dt-bindings/soc/ti,sci_pm_domain.h>

/**
 * struct ti_sci_power_domain_data - pm domain controller information structure
 * @sci: TI SCI handle used for communication with system controller
 */
struct ti_sci_power_domain_data {
	const struct ti_sci_handle *sci;
};

static int ti_sci_power_domain_probe(struct udevice *dev)
{
	struct ti_sci_power_domain_data *data = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	if (!data)
		return -ENOMEM;

	/* Store handle for communication with the system controller */
	data->sci = ti_sci_get_handle(dev);
	if (IS_ERR(data->sci))
		return PTR_ERR(data->sci);

	return 0;
}

static int ti_sci_power_domain_on(struct power_domain *pd)
{
	struct ti_sci_power_domain_data *data = dev_get_priv(pd->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_dev_ops *dops = &sci->ops.dev_ops;
	u8 flags = (uintptr_t)pd->priv;
	int ret;

	debug("%s(pd=%p)\n", __func__, pd);

	if (flags & TI_SCI_PD_EXCLUSIVE)
		ret = dops->get_device_exclusive(sci, pd->id);
	else
		ret = dops->get_device(sci, pd->id);

	if (ret)
		dev_err(pd->dev, "%s: get_device(%lu) failed (%d)\n",
			__func__, pd->id, ret);

	return ret;
}

static int ti_sci_power_domain_off(struct power_domain *pd)
{
	struct ti_sci_power_domain_data *data = dev_get_priv(pd->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_dev_ops *dops = &sci->ops.dev_ops;
	int ret;

	debug("%s(pd=%p)\n", __func__, pd);

	ret = dops->put_device(sci, pd->id);
	if (ret)
		dev_err(pd->dev, "%s: put_device(%lu) failed (%d)\n",
			__func__, pd->id, ret);

	return ret;
}

static int ti_sci_power_domain_of_xlate(struct power_domain *pd,
					struct ofnode_phandle_args *args)
{
	u8 flags;

	debug("%s(power_domain=%p)\n", __func__, pd);

	if (args->args_count < 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	pd->id = args->args[0];
	/* By default request for device exclusive */
	flags = TI_SCI_PD_EXCLUSIVE;
	if (args->args_count == 2)
		flags = args->args[1] & TI_SCI_PD_EXCLUSIVE;
	pd->priv = (void *)(uintptr_t)flags;

	return 0;
}

static const struct udevice_id ti_sci_power_domain_of_match[] = {
	{ .compatible = "ti,sci-pm-domain" },
	{ /* sentinel */ }
};

static struct power_domain_ops ti_sci_power_domain_ops = {
	.on = ti_sci_power_domain_on,
	.off = ti_sci_power_domain_off,
	.of_xlate = ti_sci_power_domain_of_xlate,
};

U_BOOT_DRIVER(ti_sci_pm_domains) = {
	.name = "ti-sci-pm-domains",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = ti_sci_power_domain_of_match,
	.probe = ti_sci_power_domain_probe,
	.priv_auto	= sizeof(struct ti_sci_power_domain_data),
	.ops = &ti_sci_power_domain_ops,
};

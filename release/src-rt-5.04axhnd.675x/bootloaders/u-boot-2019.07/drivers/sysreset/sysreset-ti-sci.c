// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface (TI SCI) system reset driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <linux/soc/ti/ti_sci_protocol.h>

/**
 * struct ti_sci_sysreset_data - sysreset controller information structure
 * @sci: TI SCI handle used for communication with system controller
 */
struct ti_sci_sysreset_data {
	const struct ti_sci_handle *sci;
};

static int ti_sci_sysreset_probe(struct udevice *dev)
{
	struct ti_sci_sysreset_data *data = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	if (!data)
		return -ENOMEM;

	/* Store handle for communication with the system controller */
	data->sci = ti_sci_get_handle(dev);
	if (IS_ERR(data->sci))
		return PTR_ERR(data->sci);

	return 0;
}

static int ti_sci_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct ti_sci_sysreset_data *data = dev_get_priv(dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_core_ops *cops = &sci->ops.core_ops;
	int ret;

	debug("%s(dev=%p, type=%d)\n", __func__, dev, type);

	ret = cops->reboot_device(sci);
	if (ret)
		dev_err(rst->dev, "%s: reboot_device failed (%d)\n",
			__func__, ret);

	return ret;
}

static struct sysreset_ops ti_sci_sysreset_ops = {
	.request = ti_sci_sysreset_request,
};

static const struct udevice_id ti_sci_sysreset_of_match[] = {
	{ .compatible = "ti,sci-sysreset", },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(ti_sci_sysreset) = {
	.name = "ti-sci-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = ti_sci_sysreset_of_match,
	.probe = ti_sci_sysreset_probe,
	.priv_auto_alloc_size = sizeof(struct ti_sci_sysreset_data),
	.ops = &ti_sci_sysreset_ops,
};

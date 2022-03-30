// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface (TI SCI) reset driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 *
 * Loosely based on Linux kernel reset-ti-sci.c...
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset-uclass.h>
#include <linux/soc/ti/ti_sci_protocol.h>

/**
 * struct ti_sci_reset_data - reset controller information structure
 * @sci: TI SCI handle used for communication with system controller
 */
struct ti_sci_reset_data {
	const struct ti_sci_handle *sci;
};

static int ti_sci_reset_probe(struct udevice *dev)
{
	struct ti_sci_reset_data *data = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	if (!data)
		return -ENOMEM;

	/* Store handle for communication with the system controller */
	data->sci = ti_sci_get_handle(dev);
	if (IS_ERR(data->sci))
		return PTR_ERR(data->sci);

	return 0;
}

static int ti_sci_reset_of_xlate(struct reset_ctl *rst,
				 struct ofnode_phandle_args *args)
{
	debug("%s(rst=%p, args_count=%d)\n", __func__, rst, args->args_count);

	if (args->args_count != 2) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/*
	 * On TI SCI-based devices, the reset provider id field is used as a
	 * device ID, and the data field is used as the associated reset mask.
	 */
	rst->id = args->args[0];
	rst->data = args->args[1];

	return 0;
}

static int ti_sci_reset_request(struct reset_ctl *rst)
{
	debug("%s(rst=%p)\n", __func__, rst);
	return 0;
}

static int ti_sci_reset_free(struct reset_ctl *rst)
{
	debug("%s(rst=%p)\n", __func__, rst);
	return 0;
}

/**
 * ti_sci_reset_set() - program a device's reset
 * @rst: Handle to a single reset signal
 * @assert: boolean flag to indicate assert or deassert
 *
 * This is a common internal function used to assert or deassert a device's
 * reset using the TI SCI protocol. The device's reset is asserted if the
 * @assert argument is true, or deasserted if @assert argument is false.
 * The mechanism itself is a read-modify-write procedure, the current device
 * reset register is read using a TI SCI device operation, the new value is
 * set or un-set using the reset's mask, and the new reset value written by
 * using another TI SCI device operation.
 *
 * Return: 0 for successful request, else a corresponding error value
 */
static int ti_sci_reset_set(struct reset_ctl *rst, bool assert)
{
	struct ti_sci_reset_data *data = dev_get_priv(rst->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_dev_ops *dops = &sci->ops.dev_ops;
	u32 reset_state;
	int ret;

	ret = dops->get_device_resets(sci, rst->id, &reset_state);
	if (ret) {
		dev_err(rst->dev, "%s: get_device_resets failed (%d)\n",
			__func__, ret);
		return ret;
	}

	if (assert)
		reset_state |= rst->data;
	else
		reset_state &= ~rst->data;

	ret = dops->set_device_resets(sci, rst->id, reset_state);
	if (ret) {
		dev_err(rst->dev, "%s: set_device_resets failed (%d)\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_reset_assert() - assert device reset
 * @rst: Handle to a single reset signal
 *
 * This function implements the reset driver op to assert a device's reset
 * using the TI SCI protocol. This invokes the function ti_sci_reset_set()
 * with the corresponding parameters as passed in, but with the @assert
 * argument set to true for asserting the reset.
 *
 * Return: 0 for successful request, else a corresponding error value
 */
static int ti_sci_reset_assert(struct reset_ctl *rst)
{
	debug("%s(rst=%p)\n", __func__, rst);
	return ti_sci_reset_set(rst, true);
}

/**
 * ti_sci_reset_deassert() - deassert device reset
 * @rst: Handle to a single reset signal
 *
 * This function implements the reset driver op to deassert a device's reset
 * using the TI SCI protocol. This invokes the function ti_sci_reset_set()
 * with the corresponding parameters as passed in, but with the @assert
 * argument set to false for deasserting the reset.
 *
 * Return: 0 for successful request, else a corresponding error value
 */
static int ti_sci_reset_deassert(struct reset_ctl *rst)
{
	debug("%s(rst=%p)\n", __func__, rst);
	return ti_sci_reset_set(rst, false);
}

/**
 * ti_sci_reset_status() - check device reset status
 * @rst: Handle to a single reset signal
 *
 * This function implements the reset driver op to return the status of a
 * device's reset using the TI SCI protocol. The reset register value is read
 * by invoking the TI SCI device operation .get_device_resets(), and the
 * status of the specific reset is extracted and returned using this reset's
 * reset mask.
 *
 * Return: 0 if reset is deasserted, or a non-zero value if reset is asserted
 */
static int ti_sci_reset_status(struct reset_ctl *rst)
{
	struct ti_sci_reset_data *data = dev_get_priv(rst->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_dev_ops *dops = &sci->ops.dev_ops;
	u32 reset_state;
	int ret;

	debug("%s(rst=%p)\n", __func__, rst);

	ret = dops->get_device_resets(sci, rst->id, &reset_state);
	if (ret) {
		dev_err(rst->dev, "%s: get_device_resets failed (%d)\n",
			__func__, ret);
		return ret;
	}

	return reset_state & rst->data;
}

static const struct udevice_id ti_sci_reset_of_match[] = {
	{ .compatible = "ti,sci-reset", },
	{ /* sentinel */ },
};

static struct reset_ops ti_sci_reset_ops = {
	.of_xlate = ti_sci_reset_of_xlate,
	.request = ti_sci_reset_request,
	.free = ti_sci_reset_free,
	.rst_assert = ti_sci_reset_assert,
	.rst_deassert = ti_sci_reset_deassert,
	.rst_status = ti_sci_reset_status,
};

U_BOOT_DRIVER(ti_sci_reset) = {
	.name = "ti-sci-reset",
	.id = UCLASS_RESET,
	.of_match = ti_sci_reset_of_match,
	.probe = ti_sci_reset_probe,
	.priv_auto_alloc_size = sizeof(struct ti_sci_reset_data),
	.ops = &ti_sci_reset_ops,
};

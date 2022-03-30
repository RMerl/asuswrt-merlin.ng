// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments System Control Interface (TI SCI) clock driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andreas Dannenberg <dannenberg@ti.com>
 *
 * Loosely based on Linux kernel sci-clk.c...
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <clk-uclass.h>
#include <linux/soc/ti/ti_sci_protocol.h>

/**
 * struct ti_sci_clk_data - clock controller information structure
 * @sci: TI SCI handle used for communication with system controller
 */
struct ti_sci_clk_data {
	const struct ti_sci_handle *sci;
};

static int ti_sci_clk_probe(struct udevice *dev)
{
	struct ti_sci_clk_data *data = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	if (!data)
		return -ENOMEM;

	/* Store handle for communication with the system controller */
	data->sci = ti_sci_get_handle(dev);
	if (IS_ERR(data->sci))
		return PTR_ERR(data->sci);

	return 0;
}

static int ti_sci_clk_of_xlate(struct clk *clk,
			       struct ofnode_phandle_args *args)
{
	debug("%s(clk=%p, args_count=%d)\n", __func__, clk, args->args_count);

	if (args->args_count != 2) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/*
	 * On TI SCI-based devices, the clock provider id field is used as a
	 * device ID, and the data field is used as the associated sub-ID.
	 */
	clk->id = args->args[0];
	clk->data = args->args[1];

	return 0;
}

static int ti_sci_clk_request(struct clk *clk)
{
	debug("%s(clk=%p)\n", __func__, clk);
	return 0;
}

static int ti_sci_clk_free(struct clk *clk)
{
	debug("%s(clk=%p)\n", __func__, clk);
	return 0;
}

static ulong ti_sci_clk_get_rate(struct clk *clk)
{
	struct ti_sci_clk_data *data = dev_get_priv(clk->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_clk_ops *cops = &sci->ops.clk_ops;
	u64 current_freq;
	int ret;

	debug("%s(clk=%p)\n", __func__, clk);

	ret = cops->get_freq(sci, clk->id, clk->data, &current_freq);
	if (ret) {
		dev_err(clk->dev, "%s: get_freq failed (%d)\n", __func__, ret);
		return ret;
	}

	debug("%s(current_freq=%llu)\n", __func__, current_freq);

	return current_freq;
}

static ulong ti_sci_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ti_sci_clk_data *data = dev_get_priv(clk->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_clk_ops *cops = &sci->ops.clk_ops;
	int ret;

	debug("%s(clk=%p, rate=%lu)\n", __func__, clk, rate);

	/* Ask for exact frequency by using same value for min/target/max */
	ret = cops->set_freq(sci, clk->id, clk->data, rate, rate, rate);
	if (ret)
		dev_err(clk->dev, "%s: set_freq failed (%d)\n", __func__, ret);

	return ret;
}

static int ti_sci_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct ti_sci_clk_data *data = dev_get_priv(clk->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_clk_ops *cops = &sci->ops.clk_ops;
	u8 num_parents;
	u8 parent_cid;
	int ret;

	debug("%s(clk=%p, parent=%p)\n", __func__, clk, parent);

	/* Make sure the clock parent is valid for a given device ID */
	if (clk->id != parent->id)
		return -EINVAL;

	/* Make sure clock has parents that can be set */
	ret = cops->get_num_parents(sci, clk->id, clk->data, &num_parents);
	if (ret) {
		dev_err(clk->dev, "%s: get_num_parents failed (%d)\n",
			__func__, ret);
		return ret;
	}
	if (num_parents < 2) {
		dev_err(clk->dev, "%s: clock has no settable parents!\n",
			__func__);
		return -EINVAL;
	}

	/* Make sure parent clock ID is valid */
	parent_cid = parent->data - clk->data - 1;
	if (parent_cid >= num_parents) {
		dev_err(clk->dev, "%s: invalid parent clock!\n", __func__);
		return -EINVAL;
	}

	/* Ready to proceed to configure the new clock parent */
	ret = cops->set_parent(sci, clk->id, clk->data, parent->data);
	if (ret)
		dev_err(clk->dev, "%s: set_parent failed (%d)\n", __func__,
			ret);

	return ret;
}

static int ti_sci_clk_enable(struct clk *clk)
{
	struct ti_sci_clk_data *data = dev_get_priv(clk->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_clk_ops *cops = &sci->ops.clk_ops;
	int ret;

	debug("%s(clk=%p)\n", __func__, clk);

	/*
	 * Allow the System Controller to automatically manage the state of
	 * this clock. If the device is enabled, then the clock is enabled.
	 */
	ret = cops->put_clock(sci, clk->id, clk->data);
	if (ret)
		dev_err(clk->dev, "%s: put_clock failed (%d)\n", __func__, ret);

	return ret;
}

static int ti_sci_clk_disable(struct clk *clk)
{
	struct ti_sci_clk_data *data = dev_get_priv(clk->dev);
	const struct ti_sci_handle *sci = data->sci;
	const struct ti_sci_clk_ops *cops = &sci->ops.clk_ops;
	int ret;

	debug("%s(clk=%p)\n", __func__, clk);

	/* Unconditionally disable clock, regardless of state of the device */
	ret = cops->idle_clock(sci, clk->id, clk->data);
	if (ret)
		dev_err(clk->dev, "%s: idle_clock failed (%d)\n", __func__,
			ret);

	return ret;
}

static const struct udevice_id ti_sci_clk_of_match[] = {
	{ .compatible = "ti,k2g-sci-clk" },
	{ /* sentinel */ },
};

static struct clk_ops ti_sci_clk_ops = {
	.of_xlate = ti_sci_clk_of_xlate,
	.request = ti_sci_clk_request,
	.free = ti_sci_clk_free,
	.get_rate = ti_sci_clk_get_rate,
	.set_rate = ti_sci_clk_set_rate,
	.set_parent = ti_sci_clk_set_parent,
	.enable = ti_sci_clk_enable,
	.disable = ti_sci_clk_disable,
};

U_BOOT_DRIVER(ti_sci_clk) = {
	.name = "ti-sci-clk",
	.id = UCLASS_CLK,
	.of_match = ti_sci_clk_of_match,
	.probe = ti_sci_clk_probe,
	.priv_auto_alloc_size = sizeof(struct ti_sci_clk_data),
	.ops = &ti_sci_clk_ops,
};

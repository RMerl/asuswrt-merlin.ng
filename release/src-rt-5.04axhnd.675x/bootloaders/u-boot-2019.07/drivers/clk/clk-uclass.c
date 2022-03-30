// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 * Copyright (c) 2018, Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/read.h>
#include <dt-structs.h>
#include <errno.h>

static inline const struct clk_ops *clk_dev_ops(struct udevice *dev)
{
	return (const struct clk_ops *)dev->driver->ops;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
# if CONFIG_IS_ENABLED(OF_PLATDATA)
int clk_get_by_index_platdata(struct udevice *dev, int index,
			      struct phandle_1_arg *cells, struct clk *clk)
{
	int ret;

	if (index != 0)
		return -ENOSYS;
	ret = uclass_get_device(UCLASS_CLK, 0, &clk->dev);
	if (ret)
		return ret;
	clk->id = cells[0].arg[0];

	return 0;
}
# else
static int clk_of_xlate_default(struct clk *clk,
				struct ofnode_phandle_args *args)
{
	debug("%s(clk=%p)\n", __func__, clk);

	if (args->args_count > 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = args->args[0];
	else
		clk->id = 0;

	return 0;
}

static int clk_get_by_index_tail(int ret, ofnode node,
				 struct ofnode_phandle_args *args,
				 const char *list_name, int index,
				 struct clk *clk)
{
	struct udevice *dev_clk;
	const struct clk_ops *ops;

	assert(clk);
	clk->dev = NULL;
	if (ret)
		goto err;

	ret = uclass_get_device_by_ofnode(UCLASS_CLK, args->node, &dev_clk);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	clk->dev = dev_clk;

	ops = clk_dev_ops(dev_clk);

	if (ops->of_xlate)
		ret = ops->of_xlate(clk, args);
	else
		ret = clk_of_xlate_default(clk, args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	return clk_request(dev_clk, clk);
err:
	debug("%s: Node '%s', property '%s', failed to request CLK index %d: %d\n",
	      __func__, ofnode_get_name(node), list_name, index, ret);
	return ret;
}

static int clk_get_by_indexed_prop(struct udevice *dev, const char *prop_name,
				   int index, struct clk *clk)
{
	int ret;
	struct ofnode_phandle_args args;

	debug("%s(dev=%p, index=%d, clk=%p)\n", __func__, dev, index, clk);

	assert(clk);
	clk->dev = NULL;

	ret = dev_read_phandle_with_args(dev, prop_name, "#clock-cells", 0,
					 index, &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}


	return clk_get_by_index_tail(ret, dev_ofnode(dev), &args, "clocks",
				     index > 0, clk);
}

int clk_get_by_index(struct udevice *dev, int index, struct clk *clk)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells", 0,
					 index, &args);

	return clk_get_by_index_tail(ret, dev_ofnode(dev), &args, "clocks",
				     index > 0, clk);
}

int clk_get_by_index_nodev(ofnode node, int index, struct clk *clk)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, "clocks", "#clock-cells", 0,
					     index > 0, &args);

	return clk_get_by_index_tail(ret, node, &args, "clocks",
				     index > 0, clk);
}

int clk_get_bulk(struct udevice *dev, struct clk_bulk *bulk)
{
	int i, ret, err, count;
	
	bulk->count = 0;

	count = dev_count_phandle_with_args(dev, "clocks", "#clock-cells");
	if (count < 1)
		return count;

	bulk->clks = devm_kcalloc(dev, count, sizeof(struct clk), GFP_KERNEL);
	if (!bulk->clks)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = clk_get_by_index(dev, i, &bulk->clks[i]);
		if (ret < 0)
			goto bulk_get_err;

		++bulk->count;
	}

	return 0;

bulk_get_err:
	err = clk_release_all(bulk->clks, bulk->count);
	if (err)
		debug("%s: could release all clocks for %p\n",
		      __func__, dev);

	return ret;
}

static int clk_set_default_parents(struct udevice *dev)
{
	struct clk clk, parent_clk;
	int index;
	int num_parents;
	int ret;

	num_parents = dev_count_phandle_with_args(dev, "assigned-clock-parents",
						  "#clock-cells");
	if (num_parents < 0) {
		debug("%s: could not read assigned-clock-parents for %p\n",
		      __func__, dev);
		return 0;
	}

	for (index = 0; index < num_parents; index++) {
		ret = clk_get_by_indexed_prop(dev, "assigned-clock-parents",
					      index, &parent_clk);
		/* If -ENOENT, this is a no-op entry */
		if (ret == -ENOENT)
			continue;

		if (ret) {
			debug("%s: could not get parent clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}

		ret = clk_get_by_indexed_prop(dev, "assigned-clocks",
					      index, &clk);
		if (ret) {
			debug("%s: could not get assigned clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}

		ret = clk_set_parent(&clk, &parent_clk);

		/*
		 * Not all drivers may support clock-reparenting (as of now).
		 * Ignore errors due to this.
		 */
		if (ret == -ENOSYS)
			continue;

		if (ret) {
			debug("%s: failed to reparent clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			return ret;
		}
	}

	return 0;
}

static int clk_set_default_rates(struct udevice *dev)
{
	struct clk clk;
	int index;
	int num_rates;
	int size;
	int ret = 0;
	u32 *rates = NULL;

	size = dev_read_size(dev, "assigned-clock-rates");
	if (size < 0)
		return 0;

	num_rates = size / sizeof(u32);
	rates = calloc(num_rates, sizeof(u32));
	if (!rates)
		return -ENOMEM;

	ret = dev_read_u32_array(dev, "assigned-clock-rates", rates, num_rates);
	if (ret)
		goto fail;

	for (index = 0; index < num_rates; index++) {
		/* If 0 is passed, this is a no-op */
		if (!rates[index])
			continue;

		ret = clk_get_by_indexed_prop(dev, "assigned-clocks",
					      index, &clk);
		if (ret) {
			debug("%s: could not get assigned clock %d for %s\n",
			      __func__, index, dev_read_name(dev));
			continue;
		}

		ret = clk_set_rate(&clk, rates[index]);
		if (ret < 0) {
			debug("%s: failed to set rate on clock index %d (%ld) for %s\n",
			      __func__, index, clk.id, dev_read_name(dev));
			break;
		}
	}

fail:
	free(rates);
	return ret;
}

int clk_set_defaults(struct udevice *dev)
{
	int ret;

	/* If this not in SPL and pre-reloc state, don't take any action. */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) || (gd->flags & GD_FLG_RELOC)))
		return 0;

	debug("%s(%s)\n", __func__, dev_read_name(dev));

	ret = clk_set_default_parents(dev);
	if (ret)
		return ret;

	ret = clk_set_default_rates(dev);
	if (ret < 0)
		return ret;

	return 0;
}
# endif /* OF_PLATDATA */

int clk_get_by_name(struct udevice *dev, const char *name, struct clk *clk)
{
	int index;

	debug("%s(dev=%p, name=%s, clk=%p)\n", __func__, dev, name, clk);
	clk->dev = NULL;

	index = dev_read_stringlist_search(dev, "clock-names", name);
	if (index < 0) {
		debug("fdt_stringlist_search() failed: %d\n", index);
		return index;
	}

	return clk_get_by_index(dev, index, clk);
}

int clk_release_all(struct clk *clk, int count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		debug("%s(clk[%d]=%p)\n", __func__, i, &clk[i]);

		/* check if clock has been previously requested */
		if (!clk[i].dev)
			continue;

		ret = clk_disable(&clk[i]);
		if (ret && ret != -ENOSYS)
			return ret;

		ret = clk_free(&clk[i]);
		if (ret && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

#endif /* OF_CONTROL */

int clk_request(struct udevice *dev, struct clk *clk)
{
	const struct clk_ops *ops = clk_dev_ops(dev);

	debug("%s(dev=%p, clk=%p)\n", __func__, dev, clk);

	clk->dev = dev;

	if (!ops->request)
		return 0;

	return ops->request(clk);
}

int clk_free(struct clk *clk)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p)\n", __func__, clk);

	if (!ops->free)
		return 0;

	return ops->free(clk);
}

ulong clk_get_rate(struct clk *clk)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p)\n", __func__, clk);

	if (!ops->get_rate)
		return -ENOSYS;

	return ops->get_rate(clk);
}

ulong clk_set_rate(struct clk *clk, ulong rate)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p, rate=%lu)\n", __func__, clk, rate);

	if (!ops->set_rate)
		return -ENOSYS;

	return ops->set_rate(clk, rate);
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p, parent=%p)\n", __func__, clk, parent);

	if (!ops->set_parent)
		return -ENOSYS;

	return ops->set_parent(clk, parent);
}

int clk_enable(struct clk *clk)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p)\n", __func__, clk);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(clk);
}

int clk_enable_bulk(struct clk_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = clk_enable(&bulk->clks[i]);
		if (ret < 0 && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

int clk_disable(struct clk *clk)
{
	const struct clk_ops *ops = clk_dev_ops(clk->dev);

	debug("%s(clk=%p)\n", __func__, clk);

	if (!ops->disable)
		return -ENOSYS;

	return ops->disable(clk);
}

int clk_disable_bulk(struct clk_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = clk_disable(&bulk->clks[i]);
		if (ret < 0 && ret != -ENOSYS)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(clk) = {
	.id		= UCLASS_CLK,
	.name		= "clk",
};

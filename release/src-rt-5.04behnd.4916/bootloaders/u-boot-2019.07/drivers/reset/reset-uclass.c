// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <reset.h>
#include <reset-uclass.h>

static inline struct reset_ops *reset_dev_ops(struct udevice *dev)
{
	return (struct reset_ops *)dev->driver->ops;
}

static int reset_of_xlate_default(struct reset_ctl *reset_ctl,
				  struct ofnode_phandle_args *args)
{
	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	if (args->args_count != 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	reset_ctl->id = args->args[0];

	return 0;
}

static int reset_get_by_index_tail(int ret, ofnode node,
				   struct ofnode_phandle_args *args,
				   const char *list_name, int index,
				   struct reset_ctl *reset_ctl)
{
	struct udevice *dev_reset;
	struct reset_ops *ops;

	assert(reset_ctl);
	reset_ctl->dev = NULL;
	if (ret)
		return ret;

	ret = uclass_get_device_by_ofnode(UCLASS_RESET, args->node,
					  &dev_reset);
	if (ret) {
		debug("%s: uclass_get_device_by_ofnode() failed: %d\n",
		      __func__, ret);
		debug("%s %d\n", ofnode_get_name(args->node), args->args[0]);
		return ret;
	}
	ops = reset_dev_ops(dev_reset);

	reset_ctl->dev = dev_reset;
	if (ops->of_xlate)
		ret = ops->of_xlate(reset_ctl, args);
	else
		ret = reset_of_xlate_default(reset_ctl, args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	ret = ops->request(reset_ctl);
	if (ret) {
		debug("ops->request() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

int reset_get_by_index(struct udevice *dev, int index,
		       struct reset_ctl *reset_ctl)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = dev_read_phandle_with_args(dev, "resets", "#reset-cells", 0,
					 index, &args);

	return reset_get_by_index_tail(ret, dev_ofnode(dev), &args, "resets",
				       index > 0, reset_ctl);
}

int reset_get_by_index_nodev(ofnode node, int index,
			     struct reset_ctl *reset_ctl)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, "resets", "#reset-cells", 0,
					     index > 0, &args);

	return reset_get_by_index_tail(ret, node, &args, "resets",
				       index > 0, reset_ctl);
}

int reset_get_bulk(struct udevice *dev, struct reset_ctl_bulk *bulk)
{
	int i, ret, err, count;
	
	bulk->count = 0;

	count = dev_count_phandle_with_args(dev, "resets", "#reset-cells");
	if (count < 1)
		return count;

	bulk->resets = devm_kcalloc(dev, count, sizeof(struct reset_ctl),
				    GFP_KERNEL);
	if (!bulk->resets)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = reset_get_by_index(dev, i, &bulk->resets[i]);
		if (ret < 0)
			goto bulk_get_err;

		++bulk->count;
	}

	return 0;

bulk_get_err:
	err = reset_release_all(bulk->resets, bulk->count);
	if (err)
		debug("%s: could release all resets for %p\n",
		      __func__, dev);

	return ret;
}

int reset_get_by_name(struct udevice *dev, const char *name,
		     struct reset_ctl *reset_ctl)
{
	int index;

	debug("%s(dev=%p, name=%s, reset_ctl=%p)\n", __func__, dev, name,
	      reset_ctl);
	reset_ctl->dev = NULL;

	index = dev_read_stringlist_search(dev, "reset-names", name);
	if (index < 0) {
		debug("fdt_stringlist_search() failed: %d\n", index);
		return index;
	}

	return reset_get_by_index(dev, index, reset_ctl);
}

int reset_request(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->request(reset_ctl);
}

int reset_free(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->free(reset_ctl);
}

int reset_assert(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->rst_assert(reset_ctl);
}

int reset_assert_bulk(struct reset_ctl_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = reset_assert(&bulk->resets[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int reset_deassert(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->rst_deassert(reset_ctl);
}

int reset_deassert_bulk(struct reset_ctl_bulk *bulk)
{
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = reset_deassert(&bulk->resets[i]);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int reset_status(struct reset_ctl *reset_ctl)
{
	struct reset_ops *ops = reset_dev_ops(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	return ops->rst_status(reset_ctl);
}

int reset_release_all(struct reset_ctl *reset_ctl, int count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		debug("%s(reset_ctl[%d]=%p)\n", __func__, i, &reset_ctl[i]);

		/* check if reset has been previously requested */
		if (!reset_ctl[i].dev)
			continue;

		ret = reset_assert(&reset_ctl[i]);
		if (ret)
			return ret;

		ret = reset_free(&reset_ctl[i]);
		if (ret)
			return ret;
	}

	return 0;
}

UCLASS_DRIVER(reset) = {
	.id		= UCLASS_RESET,
	.name		= "reset",
};

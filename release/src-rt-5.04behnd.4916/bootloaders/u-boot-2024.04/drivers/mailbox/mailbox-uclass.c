// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#define LOG_CATEGORY UCLASS_MAILBOX

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mailbox.h>
#include <mailbox-uclass.h>
#include <malloc.h>
#include <time.h>

static inline struct mbox_ops *mbox_dev_ops(struct udevice *dev)
{
	return (struct mbox_ops *)dev->driver->ops;
}

static int mbox_of_xlate_default(struct mbox_chan *chan,
				 struct ofnode_phandle_args *args)
{
	debug("%s(chan=%p)\n", __func__, chan);

	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	chan->id = args->args[0];

	return 0;
}

int mbox_get_by_index(struct udevice *dev, int index, struct mbox_chan *chan)
{
	struct ofnode_phandle_args args;
	int ret;
	struct udevice *dev_mbox;
	struct mbox_ops *ops;

	debug("%s(dev=%p, index=%d, chan=%p)\n", __func__, dev, index, chan);

	ret = dev_read_phandle_with_args(dev, "mboxes", "#mbox-cells", 0, index,
					 &args);
	if (ret) {
		debug("%s: dev_read_phandle_with_args failed: %d\n", __func__,
		      ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MAILBOX, args.node, &dev_mbox);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: %d\n",
		      __func__, ret);

		/* Test with parent node */
		ret = uclass_get_device_by_ofnode(UCLASS_MAILBOX,
						  ofnode_get_parent(args.node),
						  &dev_mbox);
		if (ret) {
			debug("%s: mbox node from parent failed: %d\n",
			      __func__, ret);
			return ret;
		};
	}
	ops = mbox_dev_ops(dev_mbox);

	chan->dev = dev_mbox;
	if (ops->of_xlate)
		ret = ops->of_xlate(chan, &args);
	else
		ret = mbox_of_xlate_default(chan, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	if (ops->request)
		ret = ops->request(chan);
	if (ret) {
		debug("ops->request() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

int mbox_get_by_name(struct udevice *dev, const char *name,
		     struct mbox_chan *chan)
{
	int index;

	debug("%s(dev=%p, name=%s, chan=%p)\n", __func__, dev, name, chan);

	index = dev_read_stringlist_search(dev, "mbox-names", name);
	if (index < 0) {
		debug("fdt_stringlist_search() failed: %d\n", index);
		return index;
	}

	return mbox_get_by_index(dev, index, chan);
}

int mbox_free(struct mbox_chan *chan)
{
	struct mbox_ops *ops = mbox_dev_ops(chan->dev);

	debug("%s(chan=%p)\n", __func__, chan);

	if (ops->rfree)
		return ops->rfree(chan);

	return 0;
}

int mbox_send(struct mbox_chan *chan, const void *data)
{
	struct mbox_ops *ops = mbox_dev_ops(chan->dev);

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	return ops->send(chan, data);
}

int mbox_recv(struct mbox_chan *chan, void *data, ulong timeout_us)
{
	struct mbox_ops *ops = mbox_dev_ops(chan->dev);
	ulong start_time;
	int ret;

	debug("%s(chan=%p, data=%p, timeout_us=%ld)\n", __func__, chan, data,
	      timeout_us);

	start_time = timer_get_us();
	/*
	 * Account for partial us ticks, but if timeout_us is 0, ensure we
	 * still don't wait at all.
	 */
	if (timeout_us)
		timeout_us++;

	for (;;) {
		ret = ops->recv(chan, data);
		if (ret != -ENODATA)
			return ret;
		if ((timer_get_us() - start_time) >= timeout_us)
			return -ETIMEDOUT;
	}
}

UCLASS_DRIVER(mailbox) = {
	.id		= UCLASS_MAILBOX,
	.name		= "mailbox",
};

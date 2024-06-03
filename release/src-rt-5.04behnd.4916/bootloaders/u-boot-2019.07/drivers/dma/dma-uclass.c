// SPDX-License-Identifier: GPL-2.0+
/*
 * Direct Memory Access U-Class driver
 *
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 - 2018 Texas Instruments Incorporated <www.ti.com>
 * Written by Mugunthan V N <mugunthanvnm@ti.com>
 *
 * Author: Mugunthan V N <mugunthanvnm@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/read.h>
#include <dma-uclass.h>
#include <dt-structs.h>
#include <errno.h>

#ifdef CONFIG_DMA_CHANNELS
static inline struct dma_ops *dma_dev_ops(struct udevice *dev)
{
	return (struct dma_ops *)dev->driver->ops;
}

# if CONFIG_IS_ENABLED(OF_CONTROL)
static int dma_of_xlate_default(struct dma *dma,
				struct ofnode_phandle_args *args)
{
	debug("%s(dma=%p)\n", __func__, dma);

	if (args->args_count > 1) {
		pr_err("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		dma->id = args->args[0];
	else
		dma->id = 0;

	return 0;
}

int dma_get_by_index(struct udevice *dev, int index, struct dma *dma)
{
	int ret;
	struct ofnode_phandle_args args;
	struct udevice *dev_dma;
	const struct dma_ops *ops;

	debug("%s(dev=%p, index=%d, dma=%p)\n", __func__, dev, index, dma);

	assert(dma);
	dma->dev = NULL;

	ret = dev_read_phandle_with_args(dev, "dmas", "#dma-cells", 0, index,
					 &args);
	if (ret) {
		pr_err("%s: dev_read_phandle_with_args failed: err=%d\n",
		       __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_DMA, args.node, &dev_dma);
	if (ret) {
		pr_err("%s: uclass_get_device_by_ofnode failed: err=%d\n",
		       __func__, ret);
		return ret;
	}

	dma->dev = dev_dma;

	ops = dma_dev_ops(dev_dma);

	if (ops->of_xlate)
		ret = ops->of_xlate(dma, &args);
	else
		ret = dma_of_xlate_default(dma, &args);
	if (ret) {
		pr_err("of_xlate() failed: %d\n", ret);
		return ret;
	}

	return dma_request(dev_dma, dma);
}

int dma_get_by_name(struct udevice *dev, const char *name, struct dma *dma)
{
	int index;

	debug("%s(dev=%p, name=%s, dma=%p)\n", __func__, dev, name, dma);
	dma->dev = NULL;

	index = dev_read_stringlist_search(dev, "dma-names", name);
	if (index < 0) {
		pr_err("dev_read_stringlist_search() failed: %d\n", index);
		return index;
	}

	return dma_get_by_index(dev, index, dma);
}
# endif /* OF_CONTROL */

int dma_request(struct udevice *dev, struct dma *dma)
{
	struct dma_ops *ops = dma_dev_ops(dev);

	debug("%s(dev=%p, dma=%p)\n", __func__, dev, dma);

	dma->dev = dev;

	if (!ops->request)
		return 0;

	return ops->request(dma);
}

int dma_free(struct dma *dma)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->free)
		return 0;

	return ops->free(dma);
}

int dma_enable(struct dma *dma)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->enable)
		return -ENOSYS;

	return ops->enable(dma);
}

int dma_disable(struct dma *dma)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->disable)
		return -ENOSYS;

	return ops->disable(dma);
}

int dma_prepare_rcv_buf(struct dma *dma, void *dst, size_t size)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->prepare_rcv_buf)
		return -1;

	return ops->prepare_rcv_buf(dma, dst, size);
}

int dma_receive(struct dma *dma, void **dst, void *metadata)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->receive)
		return -ENOSYS;

	return ops->receive(dma, dst, metadata);
}

int dma_send(struct dma *dma, void *src, size_t len, void *metadata)
{
	struct dma_ops *ops = dma_dev_ops(dma->dev);

	debug("%s(dma=%p)\n", __func__, dma);

	if (!ops->send)
		return -ENOSYS;

	return ops->send(dma, src, len, metadata);
}
#endif /* CONFIG_DMA_CHANNELS */

int dma_get_device(u32 transfer_type, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	for (ret = uclass_first_device(UCLASS_DMA, &dev); dev && !ret;
	     ret = uclass_next_device(&dev)) {
		struct dma_dev_priv *uc_priv;

		uc_priv = dev_get_uclass_priv(dev);
		if (uc_priv->supported & transfer_type)
			break;
	}

	if (!dev) {
		pr_err("No DMA device found that supports %x type\n",
		      transfer_type);
		return -EPROTONOSUPPORT;
	}

	*devp = dev;

	return ret;
}

int dma_memcpy(void *dst, void *src, size_t len)
{
	struct udevice *dev;
	const struct dma_ops *ops;
	int ret;

	ret = dma_get_device(DMA_SUPPORTS_MEM_TO_MEM, &dev);
	if (ret < 0)
		return ret;

	ops = device_get_ops(dev);
	if (!ops->transfer)
		return -ENOSYS;

	/* Invalidate the area, so no writeback into the RAM races with DMA */
	invalidate_dcache_range((unsigned long)dst, (unsigned long)dst +
				roundup(len, ARCH_DMA_MINALIGN));

	return ops->transfer(dev, DMA_MEM_TO_MEM, dst, src, len);
}

UCLASS_DRIVER(dma) = {
	.id		= UCLASS_DMA,
	.name		= "dma",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_auto_alloc_size = sizeof(struct dma_dev_priv),
};

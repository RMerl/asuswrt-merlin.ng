// SPDX-License-Identifier: GPL-2.0+
/*
 * Direct Memory Access U-Class Simulation driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated <www.ti.com>
 *
 * Author: Grygorii Strashko <grygorii.strashko@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/read.h>
#include <dma-uclass.h>
#include <dt-structs.h>
#include <errno.h>

#define SANDBOX_DMA_CH_CNT 3
#define SANDBOX_DMA_BUF_SIZE 1024

struct sandbox_dma_chan {
	struct sandbox_dma_dev *ud;
	char name[20];
	u32 id;
	enum dma_direction dir;
	bool in_use;
	bool enabled;
};

struct sandbox_dma_dev {
	struct device *dev;
	u32 ch_count;
	struct sandbox_dma_chan channels[SANDBOX_DMA_CH_CNT];
	uchar   buf[SANDBOX_DMA_BUF_SIZE];
	uchar	*buf_rx;
	size_t	data_len;
	u32	meta;
};

static int sandbox_dma_transfer(struct udevice *dev, int direction,
				void *dst, void *src, size_t len)
{
	memcpy(dst, src, len);

	return 0;
}

static int sandbox_dma_of_xlate(struct dma *dma,
				struct ofnode_phandle_args *args)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	debug("%s(dma id=%u)\n", __func__, args->args[0]);

	if (args->args[0] >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;

	dma->id = args->args[0];

	uc = &ud->channels[dma->id];

	if (dma->id == 1)
		uc->dir = DMA_MEM_TO_DEV;
	else if (dma->id == 2)
		uc->dir = DMA_DEV_TO_MEM;
	else
		uc->dir = DMA_MEM_TO_MEM;
	debug("%s(dma id=%lu dir=%d)\n", __func__, dma->id, uc->dir);

	return 0;
}

static int sandbox_dma_request(struct dma *dma)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;

	uc = &ud->channels[dma->id];
	if (uc->in_use)
		return -EBUSY;

	uc->in_use = true;
	debug("%s(dma id=%lu in_use=%d)\n", __func__, dma->id, uc->in_use);

	return 0;
}

static int sandbox_dma_free(struct dma *dma)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;

	uc = &ud->channels[dma->id];
	if (!uc->in_use)
		return -EINVAL;

	uc->in_use = false;
	ud->buf_rx = NULL;
	ud->data_len = 0;
	debug("%s(dma id=%lu in_use=%d)\n", __func__, dma->id, uc->in_use);

	return 0;
}

static int sandbox_dma_enable(struct dma *dma)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;

	uc = &ud->channels[dma->id];
	if (!uc->in_use)
		return -EINVAL;
	if (uc->enabled)
		return -EINVAL;

	uc->enabled = true;
	debug("%s(dma id=%lu enabled=%d)\n", __func__, dma->id, uc->enabled);

	return 0;
}

static int sandbox_dma_disable(struct dma *dma)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;

	uc = &ud->channels[dma->id];
	if (!uc->in_use)
		return -EINVAL;
	if (!uc->enabled)
		return -EINVAL;

	uc->enabled = false;
	debug("%s(dma id=%lu enabled=%d)\n", __func__, dma->id, uc->enabled);

	return 0;
}

static int sandbox_dma_send(struct dma *dma,
			    void *src, size_t len, void *metadata)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;
	if (!src || !metadata)
		return -EINVAL;

	debug("%s(dma id=%lu)\n", __func__, dma->id);

	uc = &ud->channels[dma->id];
	if (uc->dir != DMA_MEM_TO_DEV)
		return -EINVAL;
	if (!uc->in_use)
		return -EINVAL;
	if (!uc->enabled)
		return -EINVAL;
	if (len >= SANDBOX_DMA_BUF_SIZE)
		return -EINVAL;

	memcpy(ud->buf, src, len);
	ud->data_len = len;
	ud->meta = *((u32 *)metadata);

	debug("%s(dma id=%lu len=%zu meta=%08x)\n",
	      __func__, dma->id, len, ud->meta);

	return 0;
}

static int sandbox_dma_receive(struct dma *dma, void **dst, void *metadata)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);
	struct sandbox_dma_chan *uc;

	if (dma->id >= SANDBOX_DMA_CH_CNT)
		return -EINVAL;
	if (!dst || !metadata)
		return -EINVAL;

	uc = &ud->channels[dma->id];
	if (uc->dir != DMA_DEV_TO_MEM)
		return -EINVAL;
	if (!uc->in_use)
		return -EINVAL;
	if (!uc->enabled)
		return -EINVAL;
	if (!ud->data_len)
		return 0;

	if (ud->buf_rx) {
		memcpy(ud->buf_rx, ud->buf, ud->data_len);
		*dst = ud->buf_rx;
	} else {
		memcpy(*dst, ud->buf, ud->data_len);
	}

	*((u32 *)metadata) = ud->meta;

	debug("%s(dma id=%lu len=%zu meta=%08x %p)\n",
	      __func__, dma->id, ud->data_len, ud->meta, *dst);

	return ud->data_len;
}

static int sandbox_dma_prepare_rcv_buf(struct dma *dma, void *dst, size_t size)
{
	struct sandbox_dma_dev *ud = dev_get_priv(dma->dev);

	ud->buf_rx = dst;

	return 0;
}

static const struct dma_ops sandbox_dma_ops = {
	.transfer	= sandbox_dma_transfer,
	.of_xlate	= sandbox_dma_of_xlate,
	.request	= sandbox_dma_request,
	.free		= sandbox_dma_free,
	.enable		= sandbox_dma_enable,
	.disable	= sandbox_dma_disable,
	.send		= sandbox_dma_send,
	.receive	= sandbox_dma_receive,
	.prepare_rcv_buf = sandbox_dma_prepare_rcv_buf,
};

static int sandbox_dma_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sandbox_dma_dev *ud = dev_get_priv(dev);
	int i, ret = 0;

	uc_priv->supported = DMA_SUPPORTS_MEM_TO_MEM |
			     DMA_SUPPORTS_MEM_TO_DEV |
			     DMA_SUPPORTS_DEV_TO_MEM;

	ud->ch_count = SANDBOX_DMA_CH_CNT;
	ud->buf_rx = NULL;
	ud->meta = 0;
	ud->data_len = 0;

	pr_err("Number of channels: %u\n", ud->ch_count);

	for (i = 0; i < ud->ch_count; i++) {
		struct sandbox_dma_chan *uc = &ud->channels[i];

		uc->ud = ud;
		uc->id = i;
		sprintf(uc->name, "DMA chan%d\n", i);
		uc->in_use = false;
		uc->enabled = false;
	}

	return ret;
}

static const struct udevice_id sandbox_dma_ids[] = {
	{ .compatible = "sandbox,dma" },
	{ }
};

U_BOOT_DRIVER(sandbox_dma) = {
	.name	= "sandbox-dma",
	.id	= UCLASS_DMA,
	.of_match = sandbox_dma_ids,
	.ops	= &sandbox_dma_ops,
	.probe = sandbox_dma_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_dma_dev),
};

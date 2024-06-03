// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include "virtio_blk.h"

struct virtio_blk_priv {
	struct virtqueue *vq;
};

static ulong virtio_blk_do_req(struct udevice *dev, u64 sector,
			       lbaint_t blkcnt, void *buffer, u32 type)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	unsigned int num_out = 0, num_in = 0;
	struct virtio_sg *sgs[3];
	u8 status;
	int ret;

	struct virtio_blk_outhdr out_hdr = {
		.type = cpu_to_virtio32(dev, type),
		.sector = cpu_to_virtio64(dev, sector),
	};
	struct virtio_sg hdr_sg = { &out_hdr, sizeof(out_hdr) };
	struct virtio_sg data_sg = { buffer, blkcnt * 512 };
	struct virtio_sg status_sg = { &status, sizeof(status) };

	sgs[num_out++] = &hdr_sg;

	if (type & VIRTIO_BLK_T_OUT)
		sgs[num_out++] = &data_sg;
	else
		sgs[num_out + num_in++] = &data_sg;

	sgs[num_out + num_in++] = &status_sg;

	ret = virtqueue_add(priv->vq, sgs, num_out, num_in);
	if (ret)
		return ret;

	virtqueue_kick(priv->vq);

	while (!virtqueue_get_buf(priv->vq, NULL))
		;

	return status == VIRTIO_BLK_S_OK ? blkcnt : -EIO;
}

static ulong virtio_blk_read(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt, void *buffer)
{
	return virtio_blk_do_req(dev, start, blkcnt, buffer,
				 VIRTIO_BLK_T_IN);
}

static ulong virtio_blk_write(struct udevice *dev, lbaint_t start,
			      lbaint_t blkcnt, const void *buffer)
{
	return virtio_blk_do_req(dev, start, blkcnt, (void *)buffer,
				 VIRTIO_BLK_T_OUT);
}

static int virtio_blk_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);
	struct blk_desc *desc = dev_get_uclass_platdata(dev);
	int devnum;

	desc->if_type = IF_TYPE_VIRTIO;
	/*
	 * Initialize the devnum to -ENODEV. This is to make sure that
	 * blk_next_free_devnum() works as expected, since the default
	 * value 0 is a valid devnum.
	 */
	desc->devnum = -ENODEV;
	devnum = blk_next_free_devnum(IF_TYPE_VIRTIO);
	if (devnum < 0)
		return devnum;
	desc->devnum = devnum;
	desc->part_type = PART_TYPE_UNKNOWN;
	/*
	 * virtio mmio transport supplies string identification for us,
	 * while pci trnasport uses a 2-byte subvendor value.
	 */
	if (uc_priv->vendor >> 16)
		sprintf(desc->vendor, "%s", (char *)&uc_priv->vendor);
	else
		sprintf(desc->vendor, "%04x", uc_priv->vendor);
	desc->bdev = dev;

	/* Indicate what driver features we support */
	virtio_driver_features_init(uc_priv, NULL, 0, NULL, 0);

	return 0;
}

static int virtio_blk_probe(struct udevice *dev)
{
	struct virtio_blk_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc = dev_get_uclass_platdata(dev);
	u64 cap;
	int ret;

	ret = virtio_find_vqs(dev, 1, &priv->vq);
	if (ret)
		return ret;

	desc->blksz = 512;
	virtio_cread(dev, struct virtio_blk_config, capacity, &cap);
	desc->lba = cap;

	return 0;
}

static const struct blk_ops virtio_blk_ops = {
	.read	= virtio_blk_read,
	.write	= virtio_blk_write,
};

U_BOOT_DRIVER(virtio_blk) = {
	.name	= VIRTIO_BLK_DRV_NAME,
	.id	= UCLASS_BLK,
	.ops	= &virtio_blk_ops,
	.bind	= virtio_blk_bind,
	.probe	= virtio_blk_probe,
	.remove	= virtio_reset,
	.priv_auto_alloc_size = sizeof(struct virtio_blk_priv),
	.flags	= DM_FLAG_ACTIVE_DMA,
};

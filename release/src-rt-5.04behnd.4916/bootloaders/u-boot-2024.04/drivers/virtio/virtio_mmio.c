// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * VirtIO memory-maped I/O transport driver
 * Ported from Linux drivers/virtio/virtio_mmio.c
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <linux/bug.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/io.h>
#include "virtio_mmio.h"

static int virtio_mmio_get_config(struct udevice *udev, unsigned int offset,
				  void *buf, unsigned int len)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);
	void __iomem *base = priv->base + VIRTIO_MMIO_CONFIG;
	u8 b;
	__le16 w;
	__le32 l;

	if (priv->version == 1) {
		u8 *ptr = buf;
		int i;

		for (i = 0; i < len; i++)
			ptr[i] = readb(base + offset + i);

		return 0;
	}

	switch (len) {
	case 1:
		b = readb(base + offset);
		memcpy(buf, &b, sizeof(b));
		break;
	case 2:
		w = cpu_to_le16(readw(base + offset));
		memcpy(buf, &w, sizeof(w));
		break;
	case 4:
		l = cpu_to_le32(readl(base + offset));
		memcpy(buf, &l, sizeof(l));
		break;
	case 8:
		l = cpu_to_le32(readl(base + offset));
		memcpy(buf, &l, sizeof(l));
		l = cpu_to_le32(readl(base + offset + sizeof(l)));
		memcpy(buf + sizeof(l), &l, sizeof(l));
		break;
	default:
		WARN_ON(true);
	}

	return 0;
}

static int virtio_mmio_set_config(struct udevice *udev, unsigned int offset,
				  const void *buf, unsigned int len)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);
	void __iomem *base = priv->base + VIRTIO_MMIO_CONFIG;
	u8 b;
	__le16 w;
	__le32 l;

	if (priv->version == 1) {
		const u8 *ptr = buf;
		int i;

		for (i = 0; i < len; i++)
			writeb(ptr[i], base + offset + i);

		return 0;
	}

	switch (len) {
	case 1:
		memcpy(&b, buf, sizeof(b));
		writeb(b, base + offset);
		break;
	case 2:
		memcpy(&w, buf, sizeof(w));
		writew(le16_to_cpu(w), base + offset);
		break;
	case 4:
		memcpy(&l, buf, sizeof(l));
		writel(le32_to_cpu(l), base + offset);
		break;
	case 8:
		memcpy(&l, buf, sizeof(l));
		writel(le32_to_cpu(l), base + offset);
		memcpy(&l, buf + sizeof(l), sizeof(l));
		writel(le32_to_cpu(l), base + offset + sizeof(l));
		break;
	default:
		WARN_ON(true);
	}

	return 0;
}

static int virtio_mmio_generation(struct udevice *udev, u32 *counter)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	if (priv->version == 1)
		*counter = 0;
	else
		*counter = readl(priv->base + VIRTIO_MMIO_CONFIG_GENERATION);

	return 0;
}

static int virtio_mmio_get_status(struct udevice *udev, u8 *status)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	*status = readl(priv->base + VIRTIO_MMIO_STATUS) & 0xff;

	return 0;
}

static int virtio_mmio_set_status(struct udevice *udev, u8 status)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	/* We should never be setting status to 0 */
	WARN_ON(status == 0);

	writel(status, priv->base + VIRTIO_MMIO_STATUS);

	return 0;
}

static int virtio_mmio_reset(struct udevice *udev)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	/* 0 status means a reset */
	writel(0, priv->base + VIRTIO_MMIO_STATUS);

	return 0;
}

static int virtio_mmio_get_features(struct udevice *udev, u64 *features)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	writel(1, priv->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);
	*features = readl(priv->base + VIRTIO_MMIO_DEVICE_FEATURES);
	*features <<= 32;

	writel(0, priv->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);
	*features |= readl(priv->base + VIRTIO_MMIO_DEVICE_FEATURES);

	return 0;
}

static int virtio_mmio_set_features(struct udevice *udev)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* Make sure there is are no mixed devices */
	if (priv->version == 2 && uc_priv->legacy) {
		debug("New virtio-mmio devices (version 2) must provide VIRTIO_F_VERSION_1 feature!\n");
		return -EINVAL;
	}

	writel(1, priv->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
	writel((u32)(uc_priv->features >> 32),
	       priv->base + VIRTIO_MMIO_DRIVER_FEATURES);

	writel(0, priv->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
	writel((u32)uc_priv->features,
	       priv->base + VIRTIO_MMIO_DRIVER_FEATURES);

	return 0;
}

static struct virtqueue *virtio_mmio_setup_vq(struct udevice *udev,
					      unsigned int index)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);
	struct virtqueue *vq;
	unsigned int num;
	int err;

	/* Select the queue we're interested in */
	writel(index, priv->base + VIRTIO_MMIO_QUEUE_SEL);

	/* Queue shouldn't already be set up */
	if (readl(priv->base + (priv->version == 1 ?
	    VIRTIO_MMIO_QUEUE_PFN : VIRTIO_MMIO_QUEUE_READY))) {
		err = -ENOENT;
		goto error_available;
	}

	num = readl(priv->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
	if (num == 0) {
		err = -ENOENT;
		goto error_new_virtqueue;
	}

	/* Create the vring */
	vq = vring_create_virtqueue(index, num, VIRTIO_MMIO_VRING_ALIGN, udev);
	if (!vq) {
		err = -ENOMEM;
		goto error_new_virtqueue;
	}

	/* Activate the queue */
	writel(virtqueue_get_vring_size(vq),
	       priv->base + VIRTIO_MMIO_QUEUE_NUM);
	if (priv->version == 1) {
		u64 q_pfn = virtqueue_get_desc_addr(vq) >> PAGE_SHIFT;

		/*
		 * virtio-mmio v1 uses a 32bit QUEUE PFN. If we have something
		 * that doesn't fit in 32bit, fail the setup rather than
		 * pretending to be successful.
		 */
		if (q_pfn >> 32) {
			debug("platform bug: legacy virtio-mmio must not be used with RAM above 0x%llxGB\n",
			      0x1ULL << (32 + PAGE_SHIFT - 30));
			err = -E2BIG;
			goto error_bad_pfn;
		}

		writel(PAGE_SIZE, priv->base + VIRTIO_MMIO_QUEUE_ALIGN);
		writel(q_pfn, priv->base + VIRTIO_MMIO_QUEUE_PFN);
	} else {
		u64 addr;

		addr = virtqueue_get_desc_addr(vq);
		writel((u32)addr, priv->base + VIRTIO_MMIO_QUEUE_DESC_LOW);
		writel((u32)(addr >> 32),
		       priv->base + VIRTIO_MMIO_QUEUE_DESC_HIGH);

		addr = virtqueue_get_avail_addr(vq);
		writel((u32)addr, priv->base + VIRTIO_MMIO_QUEUE_AVAIL_LOW);
		writel((u32)(addr >> 32),
		       priv->base + VIRTIO_MMIO_QUEUE_AVAIL_HIGH);

		addr = virtqueue_get_used_addr(vq);
		writel((u32)addr, priv->base + VIRTIO_MMIO_QUEUE_USED_LOW);
		writel((u32)(addr >> 32),
		       priv->base + VIRTIO_MMIO_QUEUE_USED_HIGH);

		writel(1, priv->base + VIRTIO_MMIO_QUEUE_READY);
	}

	return vq;

error_bad_pfn:
	vring_del_virtqueue(vq);

error_new_virtqueue:
	if (priv->version == 1) {
		writel(0, priv->base + VIRTIO_MMIO_QUEUE_PFN);
	} else {
		writel(0, priv->base + VIRTIO_MMIO_QUEUE_READY);
		WARN_ON(readl(priv->base + VIRTIO_MMIO_QUEUE_READY));
	}

error_available:
	return ERR_PTR(err);
}

static void virtio_mmio_del_vq(struct virtqueue *vq)
{
	struct virtio_mmio_priv *priv = dev_get_priv(vq->vdev);
	unsigned int index = vq->index;

	/* Select and deactivate the queue */
	writel(index, priv->base + VIRTIO_MMIO_QUEUE_SEL);
	if (priv->version == 1) {
		writel(0, priv->base + VIRTIO_MMIO_QUEUE_PFN);
	} else {
		writel(0, priv->base + VIRTIO_MMIO_QUEUE_READY);
		WARN_ON(readl(priv->base + VIRTIO_MMIO_QUEUE_READY));
	}

	vring_del_virtqueue(vq);
}

static int virtio_mmio_del_vqs(struct udevice *udev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct virtqueue *vq, *n;

	list_for_each_entry_safe(vq, n, &uc_priv->vqs, list)
		virtio_mmio_del_vq(vq);

	return 0;
}

static int virtio_mmio_find_vqs(struct udevice *udev, unsigned int nvqs,
				struct virtqueue *vqs[])
{
	int i;

	for (i = 0; i < nvqs; ++i) {
		vqs[i] = virtio_mmio_setup_vq(udev, i);
		if (IS_ERR(vqs[i])) {
			virtio_mmio_del_vqs(udev);
			return PTR_ERR(vqs[i]);
		}
	}

	return 0;
}

static int virtio_mmio_notify(struct udevice *udev, struct virtqueue *vq)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	/*
	 * We write the queue's selector into the notification register
	 * to signal the other end
	 */
	writel(vq->index, priv->base + VIRTIO_MMIO_QUEUE_NOTIFY);

	return 0;
}

static int virtio_mmio_of_to_plat(struct udevice *udev)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);

	priv->base = (void __iomem *)(ulong)dev_read_addr(udev);
	if (priv->base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static int virtio_mmio_probe(struct udevice *udev)
{
	struct virtio_mmio_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	u32 magic;

	/* Check magic value */
	magic = readl(priv->base + VIRTIO_MMIO_MAGIC_VALUE);
	if (magic != ('v' | 'i' << 8 | 'r' << 16 | 't' << 24)) {
		debug("(%s): wrong magic value 0x%08x!\n", udev->name, magic);
		return 0;
	}

	/* Check device version */
	priv->version = readl(priv->base + VIRTIO_MMIO_VERSION);
	if (priv->version < 1 || priv->version > 2) {
		debug("(%s): version %d not supported!\n",
		      udev->name, priv->version);
		return 0;
	}

	/* Check device ID */
	uc_priv->device = readl(priv->base + VIRTIO_MMIO_DEVICE_ID);
	if (uc_priv->device == 0) {
		/*
		 * virtio-mmio device with an ID 0 is a (dummy) placeholder
		 * with no function. End probing now with no error reported.
		 */
		return 0;
	}
	uc_priv->vendor = readl(priv->base + VIRTIO_MMIO_VENDOR_ID);

	if (priv->version == 1)
		writel(PAGE_SIZE, priv->base + VIRTIO_MMIO_GUEST_PAGE_SIZE);

	debug("(%s): device (%d) vendor (%08x) version (%d)\n", udev->name,
	      uc_priv->device, uc_priv->vendor, priv->version);

	return 0;
}

static const struct dm_virtio_ops virtio_mmio_ops = {
	.get_config	= virtio_mmio_get_config,
	.set_config	= virtio_mmio_set_config,
	.generation	= virtio_mmio_generation,
	.get_status	= virtio_mmio_get_status,
	.set_status	= virtio_mmio_set_status,
	.reset		= virtio_mmio_reset,
	.get_features	= virtio_mmio_get_features,
	.set_features	= virtio_mmio_set_features,
	.find_vqs	= virtio_mmio_find_vqs,
	.del_vqs	= virtio_mmio_del_vqs,
	.notify		= virtio_mmio_notify,
};

static const struct udevice_id virtio_mmio_ids[] = {
	{ .compatible = "virtio,mmio" },
	{ }
};

U_BOOT_DRIVER(virtio_mmio) = {
	.name	= "virtio-mmio",
	.id	= UCLASS_VIRTIO,
	.of_match = virtio_mmio_ids,
	.ops	= &virtio_mmio_ops,
	.probe	= virtio_mmio_probe,
	.of_to_plat = virtio_mmio_of_to_plat,
	.priv_auto	= sizeof(struct virtio_mmio_priv),
};

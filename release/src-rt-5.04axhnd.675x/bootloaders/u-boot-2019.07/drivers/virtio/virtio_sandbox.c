// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * VirtIO Sandbox transport driver, for testing purpose only
 */

#include <common.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <linux/compat.h>
#include <linux/io.h>

struct virtio_sandbox_priv {
	u8 id;
	u8 status;
	u64 device_features;
	u64 driver_features;
	ulong queue_desc;
	ulong queue_available;
	ulong queue_used;
};

static int virtio_sandbox_get_config(struct udevice *udev, unsigned int offset,
				     void *buf, unsigned int len)
{
	return 0;
}

static int virtio_sandbox_set_config(struct udevice *udev, unsigned int offset,
				     const void *buf, unsigned int len)
{
	return 0;
}

static int virtio_sandbox_get_status(struct udevice *udev, u8 *status)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);

	*status = priv->status;

	return 0;
}

static int virtio_sandbox_set_status(struct udevice *udev, u8 status)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);

	/* We should never be setting status to 0 */
	WARN_ON(status == 0);

	priv->status = status;

	return 0;
}

static int virtio_sandbox_reset(struct udevice *udev)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);

	/* 0 status means a reset */
	priv->status = 0;

	return 0;
}

static int virtio_sandbox_get_features(struct udevice *udev, u64 *features)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);

	*features = priv->device_features;

	return 0;
}

static int virtio_sandbox_set_features(struct udevice *udev)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	priv->driver_features = uc_priv->features;

	return 0;
}

static struct virtqueue *virtio_sandbox_setup_vq(struct udevice *udev,
						 unsigned int index)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);
	struct virtqueue *vq;
	ulong addr;
	int err;

	/* Create the vring */
	vq = vring_create_virtqueue(index, 4, 4096, udev);
	if (!vq) {
		err = -ENOMEM;
		goto error_new_virtqueue;
	}

	addr = virtqueue_get_desc_addr(vq);
	priv->queue_desc = addr;

	addr = virtqueue_get_avail_addr(vq);
	priv->queue_available = addr;

	addr = virtqueue_get_used_addr(vq);
	priv->queue_used = addr;

	return vq;

error_new_virtqueue:
	return ERR_PTR(err);
}

static void virtio_sandbox_del_vq(struct virtqueue *vq)
{
	vring_del_virtqueue(vq);
}

static int virtio_sandbox_del_vqs(struct udevice *udev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct virtqueue *vq, *n;

	list_for_each_entry_safe(vq, n, &uc_priv->vqs, list)
		virtio_sandbox_del_vq(vq);

	return 0;
}

static int virtio_sandbox_find_vqs(struct udevice *udev, unsigned int nvqs,
				   struct virtqueue *vqs[])
{
	int i;

	for (i = 0; i < nvqs; ++i) {
		vqs[i] = virtio_sandbox_setup_vq(udev, i);
		if (IS_ERR(vqs[i])) {
			virtio_sandbox_del_vqs(udev);
			return PTR_ERR(vqs[i]);
		}
	}

	return 0;
}

static int virtio_sandbox_notify(struct udevice *udev, struct virtqueue *vq)
{
	return 0;
}

static int virtio_sandbox_probe(struct udevice *udev)
{
	struct virtio_sandbox_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* fake some information for testing */
	priv->device_features = VIRTIO_F_VERSION_1;
	uc_priv->device = VIRTIO_ID_BLOCK;
	uc_priv->vendor = ('u' << 24) | ('b' << 16) | ('o' << 8) | 't';

	return 0;
}

/* check virtio device driver's remove routine was called to reset the device */
static int virtio_sandbox_child_post_remove(struct udevice *vdev)
{
	u8 status;

	virtio_get_status(vdev, &status);
	if (status)
		panic("virtio device was not reset\n");

	return 0;
}

static const struct dm_virtio_ops virtio_sandbox1_ops = {
	.get_config	= virtio_sandbox_get_config,
	.set_config	= virtio_sandbox_set_config,
	.get_status	= virtio_sandbox_get_status,
	.set_status	= virtio_sandbox_set_status,
	.reset		= virtio_sandbox_reset,
	.get_features	= virtio_sandbox_get_features,
	.set_features	= virtio_sandbox_set_features,
	.find_vqs	= virtio_sandbox_find_vqs,
	.del_vqs	= virtio_sandbox_del_vqs,
	.notify		= virtio_sandbox_notify,
};

static const struct udevice_id virtio_sandbox1_ids[] = {
	{ .compatible = "sandbox,virtio1" },
	{ }
};

U_BOOT_DRIVER(virtio_sandbox1) = {
	.name	= "virtio-sandbox1",
	.id	= UCLASS_VIRTIO,
	.of_match = virtio_sandbox1_ids,
	.ops	= &virtio_sandbox1_ops,
	.probe	= virtio_sandbox_probe,
	.child_post_remove = virtio_sandbox_child_post_remove,
	.priv_auto_alloc_size = sizeof(struct virtio_sandbox_priv),
};

/* this one without notify op */
static const struct dm_virtio_ops virtio_sandbox2_ops = {
	.get_config	= virtio_sandbox_get_config,
	.set_config	= virtio_sandbox_set_config,
	.get_status	= virtio_sandbox_get_status,
	.set_status	= virtio_sandbox_set_status,
	.reset		= virtio_sandbox_reset,
	.get_features	= virtio_sandbox_get_features,
	.set_features	= virtio_sandbox_set_features,
	.find_vqs	= virtio_sandbox_find_vqs,
	.del_vqs	= virtio_sandbox_del_vqs,
};

static const struct udevice_id virtio_sandbox2_ids[] = {
	{ .compatible = "sandbox,virtio2" },
	{ }
};

U_BOOT_DRIVER(virtio_sandbox2) = {
	.name	= "virtio-sandbox2",
	.id	= UCLASS_VIRTIO,
	.of_match = virtio_sandbox2_ids,
	.ops	= &virtio_sandbox2_ops,
	.probe	= virtio_sandbox_probe,
	.priv_auto_alloc_size = sizeof(struct virtio_sandbox_priv),
};

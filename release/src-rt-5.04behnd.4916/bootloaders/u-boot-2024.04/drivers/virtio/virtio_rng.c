// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <rng.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>

#define BUFFER_SIZE	16UL

struct virtio_rng_priv {
	struct virtqueue *rng_vq;
};

static int virtio_rng_read(struct udevice *dev, void *data, size_t len)
{
	int ret;
	unsigned int rsize = 1;
	unsigned char buf[BUFFER_SIZE] __aligned(4);
	unsigned char *ptr = data;
	struct virtio_sg sg;
	struct virtio_sg *sgs[1];
	struct virtio_rng_priv *priv = dev_get_priv(dev);

	while (len) {
		sg.addr = buf;
		/*
		 * Work around implementations which always return 8 bytes
		 * less than requested, down to 0 bytes, which would
		 * cause an endless loop otherwise.
		 */
		sg.length = min(rsize ? len : len + 8, sizeof(buf));
		sgs[0] = &sg;

		ret = virtqueue_add(priv->rng_vq, sgs, 0, 1);
		if (ret)
			return ret;

		virtqueue_kick(priv->rng_vq);

		while (!virtqueue_get_buf(priv->rng_vq, &rsize))
			;

		if (rsize > sg.length)
			return -EIO;

		memcpy(ptr, buf, rsize);
		len -= rsize;
		ptr += rsize;
	}

	return 0;
}

static int virtio_rng_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);

	/* Indicate what driver features we support */
	virtio_driver_features_init(uc_priv, NULL, 0, NULL, 0);

	return 0;
}

static int virtio_rng_probe(struct udevice *dev)
{
	struct virtio_rng_priv *priv = dev_get_priv(dev);
	int ret;

	ret = virtio_find_vqs(dev, 1, &priv->rng_vq);
	if (ret < 0) {
		debug("%s: virtio_find_vqs failed\n", __func__);
		return ret;
	}

	return 0;
}

static const struct dm_rng_ops virtio_rng_ops = {
	.read	= virtio_rng_read,
};

U_BOOT_DRIVER(virtio_rng) = {
	.name	= VIRTIO_RNG_DRV_NAME,
	.id	= UCLASS_RNG,
	.bind	= virtio_rng_bind,
	.probe	= virtio_rng_probe,
	.remove = virtio_reset,
	.ops	= &virtio_rng_ops,
	.priv_auto	= sizeof(struct virtio_rng_priv),
	.flags	= DM_FLAG_ACTIVE_DMA,
};

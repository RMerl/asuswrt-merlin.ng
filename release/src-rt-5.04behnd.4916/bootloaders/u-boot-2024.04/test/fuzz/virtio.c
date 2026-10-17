/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#include <common.h>
#include <dm.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <test/fuzz.h>

static int fuzz_vring(const uint8_t *data, size_t size)
{
	struct udevice *bus, *dev;
	struct virtio_dev_priv *uc_priv;
	struct virtqueue *vq;
	struct virtio_sg sg[2];
	struct virtio_sg *sgs[2];
	unsigned int len;
	u8 buffer[2][32];

	/* hackily hardcode vring sizes */
	size_t num = 4;
	size_t desc_size = (sizeof(struct vring_desc) * num);
	size_t avail_size = (3 + num) * sizeof(u16);
	size_t used_size = (3 * sizeof(u16)) + (sizeof(struct vring_used_elem) * num);

	if (size < (desc_size + avail_size + used_size))
		return 0;

	/* check probe success */
	if (uclass_first_device_err(UCLASS_VIRTIO, &bus))
		panic("Could not find virtio bus\n");

	/* check the child virtio-rng device is bound */
	if (device_find_first_child(bus, &dev) || !dev)
		panic("Could not find virtio device\n");

	/*
	 * fake the virtio device probe by filling in uc_priv->vdev
	 * which is used by virtio_find_vqs/virtio_del_vqs.
	 */
	uc_priv = dev_get_uclass_priv(bus);
	uc_priv->vdev = dev;

	/* prepare the scatter-gather buffer */
	sg[0].addr = buffer[0];
	sg[0].length = sizeof(buffer[0]);
	sg[1].addr = buffer[1];
	sg[1].length = sizeof(buffer[1]);
	sgs[0] = &sg[0];
	sgs[1] = &sg[1];

	if (virtio_find_vqs(dev, 1, &vq))
		panic("Could not find vqs\n");
	if (virtqueue_add(vq, sgs, 0, 1))
		panic("Could not add to virtqueue\n");
	/* Simulate device writing to vring */
	memcpy(vq->vring.desc, data, desc_size);
	memcpy(vq->vring.avail, data + desc_size, avail_size);
	memcpy(vq->vring.used, data + desc_size + avail_size, used_size);
	/* Make sure there is a response */
	if (vq->vring.used->idx == 0)
		vq->vring.used->idx = 1;
	virtqueue_get_buf(vq, &len);
	if (virtio_del_vqs(dev))
		panic("Could not delete vqs\n");

	return 0;
}
FUZZ_TEST(fuzz_vring, 0);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the virtio uclass */
static int dm_test_virtio_base(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	u8 status;

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_VIRTIO, &bus));
	ut_assertnonnull(bus);

	/* check the child virtio-rng device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);
	ut_asserteq_str("virtio-rng#0", dev->name);

	/* check driver status */
	ut_assertok(virtio_get_status(dev, &status));
	ut_asserteq(VIRTIO_CONFIG_S_ACKNOWLEDGE, status);

	/* probe the virtio-rng driver */
	ut_assertok(device_probe(dev));

	/* check the device was reset and the driver picked up the device */
	ut_assertok(virtio_get_status(dev, &status));
	ut_asserteq(VIRTIO_CONFIG_S_DRIVER |
		    VIRTIO_CONFIG_S_DRIVER_OK |
		    VIRTIO_CONFIG_S_FEATURES_OK, status);

	return 0;
}
DM_TEST(dm_test_virtio_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test all of the virtio uclass ops */
static int dm_test_virtio_all_ops(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	struct virtio_dev_priv *uc_priv;
	uint offset = 0, len = 0, nvqs = 1;
	void *buffer = NULL;
	u8 status;
	u32 counter;
	u64 features;
	struct virtqueue *vqs[2];

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_VIRTIO, &bus));
	ut_assertnonnull(bus);

	/* check the child virtio-rng device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);

	/*
	 * fake the virtio device probe by filling in uc_priv->vdev
	 * which is used by virtio_find_vqs/virtio_del_vqs.
	 */
	uc_priv = dev_get_uclass_priv(bus);
	ut_assertnonnull(uc_priv);
	uc_priv->vdev = dev;

	/* test virtio_xxx APIs */
	ut_assertok(virtio_get_config(dev, offset, buffer, len));
	ut_assertok(virtio_set_config(dev, offset, buffer, len));
	ut_asserteq(-ENOSYS, virtio_generation(dev, &counter));
	ut_assertok(virtio_set_status(dev, VIRTIO_CONFIG_S_DRIVER_OK));
	ut_assertok(virtio_get_status(dev, &status));
	ut_asserteq(VIRTIO_CONFIG_S_DRIVER_OK, status);
	ut_assertok(virtio_reset(dev));
	ut_assertok(virtio_get_status(dev, &status));
	ut_asserteq(0, status);
	ut_assertok(virtio_get_features(dev, &features));
	ut_asserteq_64(BIT_ULL(VIRTIO_F_VERSION_1), features);
	ut_assertok(virtio_set_features(dev));
	ut_assertok(virtio_find_vqs(dev, nvqs, vqs));
	ut_assertok(virtio_notify(dev, vqs[0]));
	ut_assertok(virtio_del_vqs(dev));

	return 0;
}
DM_TEST(dm_test_virtio_all_ops, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test removal of virtio device driver */
static int dm_test_virtio_remove(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	u8 status;

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_VIRTIO, &bus));
	ut_assertnonnull(bus);

	/* check the child virtio-rng device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);

	/* set driver status to VIRTIO_CONFIG_S_DRIVER_OK */
	ut_assertok(virtio_set_status(dev, VIRTIO_CONFIG_S_DRIVER_OK));

	/* check the device can be successfully removed */
	dev_or_flags(dev, DM_FLAG_ACTIVATED);
	ut_asserteq(-EKEYREJECTED, device_remove(bus, DM_REMOVE_ACTIVE_ALL));

	ut_asserteq(false, device_active(dev));
	virtio_get_status(dev, &status);
	ut_assertok(status);

	return 0;
}
DM_TEST(dm_test_virtio_remove, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test all of the virtio ring */
static int dm_test_virtio_ring(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	struct virtio_dev_priv *uc_priv;
	struct virtqueue *vq;
	struct virtio_sg sg[2];
	struct virtio_sg *sgs[2];
	unsigned int len;
	u8 buffer[2][32];

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_VIRTIO, &bus));
	ut_assertnonnull(bus);

	/* check the child virtio-blk device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);

	/*
	 * fake the virtio device probe by filling in uc_priv->vdev
	 * which is used by virtio_find_vqs/virtio_del_vqs.
	 */
	uc_priv = dev_get_uclass_priv(bus);
	ut_assertnonnull(uc_priv);
	uc_priv->vdev = dev;

	/* prepare the scatter-gather buffer */
	sg[0].addr = buffer[0];
	sg[0].length = sizeof(buffer[0]);
	sg[1].addr = buffer[1];
	sg[1].length = sizeof(buffer[1]);
	sgs[0] = &sg[0];
	sgs[1] = &sg[1];

	/* read a buffer and report written size from device */
	ut_assertok(virtio_find_vqs(dev, 1, &vq));
	ut_assertok(virtqueue_add(vq, sgs, 0, 1));
	vq->vring.used->idx = 1;
	vq->vring.used->ring[0].id = 0;
	vq->vring.used->ring[0].len = 0x53355885;
	ut_asserteq_ptr(buffer, virtqueue_get_buf(vq, &len));
	ut_asserteq(0x53355885, len);
	ut_assertok(virtio_del_vqs(dev));

	/* rejects used descriptors that aren't a chain head */
	ut_assertok(virtio_find_vqs(dev, 1, &vq));
	ut_assertok(virtqueue_add(vq, sgs, 0, 2));
	vq->vring.used->idx = 1;
	vq->vring.used->ring[0].id = 1;
	vq->vring.used->ring[0].len = 0x53355885;
	ut_assertnull(virtqueue_get_buf(vq, &len));
	ut_assertok(virtio_del_vqs(dev));

	/* device changes to descriptor are ignored */
	ut_assertok(virtio_find_vqs(dev, 1, &vq));
	ut_assertok(virtqueue_add(vq, sgs, 0, 1));
	vq->vring.desc[0].addr = cpu_to_virtio64(dev, 0xbadbad11);
	vq->vring.desc[0].len = cpu_to_virtio32(dev, 0x11badbad);
	vq->vring.desc[0].flags = cpu_to_virtio16(dev, VRING_DESC_F_NEXT);
	vq->vring.desc[0].next = cpu_to_virtio16(dev, U16_MAX);
	vq->vring.used->idx = 1;
	vq->vring.used->ring[0].id = 0;
	vq->vring.used->ring[0].len = 6;
	ut_asserteq_ptr(buffer, virtqueue_get_buf(vq, &len));
	ut_asserteq(6, len);
	ut_assertok(virtio_del_vqs(dev));

	return 0;
}
DM_TEST(dm_test_virtio_ring, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

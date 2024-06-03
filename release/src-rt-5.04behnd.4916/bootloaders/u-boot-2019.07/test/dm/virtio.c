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
#include <dm/uclass-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <test/ut.h>

/* Basic test of the virtio uclass */
static int dm_test_virtio_base(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	u8 status;

	/* check probe success */
	ut_assertok(uclass_first_device(UCLASS_VIRTIO, &bus));

	/* check the child virtio-blk device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertok(strcmp(dev->name, "virtio-blk#0"));

	/* check driver status */
	ut_assertok(virtio_get_status(dev, &status));
	ut_asserteq(VIRTIO_CONFIG_S_ACKNOWLEDGE, status);

	return 0;
}
DM_TEST(dm_test_virtio_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
	ut_assertok(uclass_first_device(UCLASS_VIRTIO, &bus));

	/* check the child virtio-blk device is bound */
	ut_assertok(device_find_first_child(bus, &dev));

	/*
	 * fake the virtio device probe by filling in uc_priv->vdev
	 * which is used by virtio_find_vqs/virtio_del_vqs.
	 */
	uc_priv = dev_get_uclass_priv(bus);
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
	ut_asserteq(VIRTIO_F_VERSION_1, features);
	ut_assertok(virtio_set_features(dev));
	ut_assertok(virtio_find_vqs(dev, nvqs, vqs));
	ut_assertok(virtio_del_vqs(dev));
	ut_assertok(virtio_notify(dev, vqs[0]));

	return 0;
}
DM_TEST(dm_test_virtio_all_ops, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test of the virtio driver that does not have required driver ops */
static int dm_test_virtio_missing_ops(struct unit_test_state *uts)
{
	struct udevice *bus;

	/* find the virtio device */
	ut_assertok(uclass_find_device(UCLASS_VIRTIO, 1, &bus));

	/*
	 * Probe the device should fail with error -ENOENT.
	 * See ops check in virtio_uclass_pre_probe().
	 */
	ut_asserteq(-ENOENT, device_probe(bus));

	return 0;
}
DM_TEST(dm_test_virtio_missing_ops, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test removal of virtio device driver */
static int dm_test_virtio_remove(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;

	/* check probe success */
	ut_assertok(uclass_first_device(UCLASS_VIRTIO, &bus));

	/* check the child virtio-blk device is bound */
	ut_assertok(device_find_first_child(bus, &dev));

	/* set driver status to VIRTIO_CONFIG_S_DRIVER_OK */
	ut_assertok(virtio_set_status(dev, VIRTIO_CONFIG_S_DRIVER_OK));

	/* check the device can be successfully removed */
	dev->flags |= DM_FLAG_ACTIVATED;
	ut_assertok(device_remove(bus, DM_REMOVE_ACTIVE_ALL));

	return 0;
}
DM_TEST(dm_test_virtio_remove, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

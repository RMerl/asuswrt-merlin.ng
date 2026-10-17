// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Google, Inc.
 * Written by Andrew Scull <ascull@google.com>
 */

#include <common.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <rng.h>
#include <test/test.h>
#include <test/ut.h>

/* This is a brittle means of getting access to the virtqueue */
struct virtio_rng_priv {
	struct virtqueue *rng_vq;
};

/* Test the virtio-rng driver validates the used size */
static int dm_test_virtio_rng_check_len(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	struct virtio_rng_priv *priv;
	u8 buffer[16];

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_VIRTIO, &bus));
	ut_assertnonnull(bus);

	/* check the child virtio-rng device is bound */
	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);

	/* probe the virtio-rng driver */
	ut_assertok(device_probe(dev));

	/* simulate the device returning the buffer with too much data */
	priv = dev_get_priv(dev);
	priv->rng_vq->vring.used->idx = 1;
	priv->rng_vq->vring.used->ring[0].id = 0;
	priv->rng_vq->vring.used->ring[0].len = U32_MAX;

	/* check the driver gracefully handles the error */
	ut_asserteq(-EIO, dm_rng_read(dev, buffer, sizeof(buffer)));

	return 0;
}
DM_TEST(dm_test_virtio_rng_check_len, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

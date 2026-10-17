// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/test.h>
#include <linux/list.h>
#include <test/test.h>
#include <test/ut.h>

int test_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct test_ops *ops = device_get_ops(dev);

	if (!ops->ping)
		return -ENOSYS;

	return ops->ping(dev, pingval, pingret);
}

static int test_post_bind(struct udevice *dev)
{
	struct unit_test_state *uts = test_get_state();
	struct dm_test_perdev_uc_pdata *uc_pdata;

	dm_testdrv_op_count[DM_TEST_OP_POST_BIND]++;
	ut_assert(!device_active(dev));

	uc_pdata = dev_get_uclass_plat(dev);
	ut_assert(uc_pdata);

	uc_pdata->intval1 = TEST_UC_PDATA_INTVAL1;
	uc_pdata->intval2 = TEST_UC_PDATA_INTVAL2;
	uc_pdata->intval3 = TEST_UC_PDATA_INTVAL3;

	return 0;
}

static int test_pre_unbind(struct udevice *dev)
{
	dm_testdrv_op_count[DM_TEST_OP_PRE_UNBIND]++;

	return 0;
}

static int test_pre_probe(struct udevice *dev)
{
	struct dm_test_uclass_perdev_priv *priv = dev_get_uclass_priv(dev);
	struct unit_test_state *uts = test_get_state();

	dm_testdrv_op_count[DM_TEST_OP_PRE_PROBE]++;
	ut_assert(priv);
	ut_assert(device_active(dev));

	return 0;
}

static int test_post_probe(struct udevice *dev)
{
	struct unit_test_state *uts = test_get_state();
	struct udevice *prev = list_entry(dev->uclass_node.prev,
					    struct udevice, uclass_node);

	struct dm_test_uclass_perdev_priv *priv = dev_get_uclass_priv(dev);
	struct uclass *uc = dev->uclass;

	dm_testdrv_op_count[DM_TEST_OP_POST_PROBE]++;
	ut_assert(priv);
	ut_assert(device_active(dev));
	priv->base_add = 0;
	if (uts->skip_post_probe)
		return 0;
	if (&prev->uclass_node != &uc->dev_head) {
		struct dm_test_uclass_perdev_priv *prev_uc_priv
				= dev_get_uclass_priv(prev);
		struct dm_test_pdata *pdata = dev_get_plat(prev);

		ut_assert(pdata);
		ut_assert(prev_uc_priv);
		priv->base_add = prev_uc_priv->base_add + pdata->ping_add;
	}

	return 0;
}

static int test_pre_remove(struct udevice *dev)
{
	dm_testdrv_op_count[DM_TEST_OP_PRE_REMOVE]++;

	return 0;
}

static int test_init(struct uclass *uc)
{
	struct unit_test_state *uts = test_get_state();

	dm_testdrv_op_count[DM_TEST_OP_INIT]++;
	ut_assert(uclass_get_priv(uc));

	return 0;
}

static int test_destroy(struct uclass *uc)
{
	dm_testdrv_op_count[DM_TEST_OP_DESTROY]++;

	return 0;
}

UCLASS_DRIVER(test) = {
	.name		= "test",
	.id		= UCLASS_TEST,
	.post_bind	= test_post_bind,
	.pre_unbind	= test_pre_unbind,
	.pre_probe	= test_pre_probe,
	.post_probe	= test_post_probe,
	.pre_remove	= test_pre_remove,
	.init		= test_init,
	.destroy	= test_destroy,
	.priv_auto	= sizeof(struct dm_test_uclass_priv),
	.per_device_auto	= sizeof(struct dm_test_uclass_perdev_priv),
	.per_device_plat_auto	= sizeof(struct dm_test_perdev_uc_pdata),
};

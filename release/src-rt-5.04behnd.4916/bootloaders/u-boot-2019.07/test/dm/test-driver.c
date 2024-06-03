// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <dm/test.h>
#include <test/ut.h>
#include <asm/io.h>

int dm_testdrv_op_count[DM_TEST_OP_COUNT];
static struct unit_test_state *uts = &global_dm_test_state;

static int testdrv_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct dm_test_pdata *pdata = dev_get_platdata(dev);
	struct dm_test_priv *priv = dev_get_priv(dev);

	*pingret = pingval + pdata->ping_add;
	priv->ping_total += *pingret;

	return 0;
}

static const struct test_ops test_ops = {
	.ping = testdrv_ping,
};

static int test_bind(struct udevice *dev)
{
	/* Private data should not be allocated */
	ut_assert(!dev_get_priv(dev));

	dm_testdrv_op_count[DM_TEST_OP_BIND]++;
	return 0;
}

static int test_probe(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	/* Private data should be allocated */
	ut_assert(priv);

	dm_testdrv_op_count[DM_TEST_OP_PROBE]++;
	priv->ping_total += DM_TEST_START_TOTAL;
	return 0;
}

static int test_remove(struct udevice *dev)
{
	/* Private data should still be allocated */
	ut_assert(dev_get_priv(dev));

	dm_testdrv_op_count[DM_TEST_OP_REMOVE]++;
	return 0;
}

static int test_unbind(struct udevice *dev)
{
	/* Private data should not be allocated */
	ut_assert(!dev->priv);

	dm_testdrv_op_count[DM_TEST_OP_UNBIND]++;
	return 0;
}

U_BOOT_DRIVER(test_drv) = {
	.name	= "test_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_ops,
	.bind	= test_bind,
	.probe	= test_probe,
	.remove	= test_remove,
	.unbind	= test_unbind,
	.priv_auto_alloc_size = sizeof(struct dm_test_priv),
};

U_BOOT_DRIVER(test2_drv) = {
	.name	= "test2_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_ops,
	.bind	= test_bind,
	.probe	= test_probe,
	.remove	= test_remove,
	.unbind	= test_unbind,
	.priv_auto_alloc_size = sizeof(struct dm_test_priv),
};

static int test_manual_drv_ping(struct udevice *dev, int pingval, int *pingret)
{
	*pingret = pingval + 2;

	return 0;
}

static const struct test_ops test_manual_ops = {
	.ping = test_manual_drv_ping,
};

static int test_manual_bind(struct udevice *dev)
{
	dm_testdrv_op_count[DM_TEST_OP_BIND]++;

	return 0;
}

static int test_manual_probe(struct udevice *dev)
{
	struct dm_test_state *dms = uts->priv;

	dm_testdrv_op_count[DM_TEST_OP_PROBE]++;
	if (!dms->force_fail_alloc)
		dev->priv = calloc(1, sizeof(struct dm_test_priv));
	if (!dev->priv)
		return -ENOMEM;

	return 0;
}

static int test_manual_remove(struct udevice *dev)
{
	dm_testdrv_op_count[DM_TEST_OP_REMOVE]++;
	return 0;
}

static int test_manual_unbind(struct udevice *dev)
{
	dm_testdrv_op_count[DM_TEST_OP_UNBIND]++;
	return 0;
}

U_BOOT_DRIVER(test_manual_drv) = {
	.name	= "test_manual_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_manual_ops,
	.bind	= test_manual_bind,
	.probe	= test_manual_probe,
	.remove	= test_manual_remove,
	.unbind	= test_manual_unbind,
};

U_BOOT_DRIVER(test_pre_reloc_drv) = {
	.name	= "test_pre_reloc_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_manual_ops,
	.bind	= test_manual_bind,
	.probe	= test_manual_probe,
	.remove	= test_manual_remove,
	.unbind	= test_manual_unbind,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(test_act_dma_drv) = {
	.name	= "test_act_dma_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_manual_ops,
	.bind	= test_manual_bind,
	.probe	= test_manual_probe,
	.remove	= test_manual_remove,
	.unbind	= test_manual_unbind,
	.flags	= DM_FLAG_ACTIVE_DMA,
};

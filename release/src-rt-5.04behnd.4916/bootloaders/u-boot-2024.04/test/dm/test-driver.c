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
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

int dm_testdrv_op_count[DM_TEST_OP_COUNT];

static int testdrv_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct dm_test_pdata *pdata = dev_get_plat(dev);
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
	struct unit_test_state *uts = test_get_state();

	/* Private data should not be allocated */
	ut_assert(!dev_get_priv(dev));

	dm_testdrv_op_count[DM_TEST_OP_BIND]++;
	return 0;
}

static int test_probe(struct udevice *dev)
{
	struct unit_test_state *uts = test_get_state();
	struct dm_test_priv *priv = dev_get_priv(dev);

	/* Private data should be allocated */
	ut_assert(priv);

	dm_testdrv_op_count[DM_TEST_OP_PROBE]++;
	priv->ping_total += DM_TEST_START_TOTAL;
	return 0;
}

static int test_remove(struct udevice *dev)
{
	struct unit_test_state *uts = test_get_state();

	/* Private data should still be allocated */
	ut_assert(dev_get_priv(dev));

	dm_testdrv_op_count[DM_TEST_OP_REMOVE]++;
	return 0;
}

static int test_unbind(struct udevice *dev)
{
	struct unit_test_state *uts = test_get_state();

	/* Private data should not be allocated */
	ut_assert(!dev_get_priv(dev));

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
	.priv_auto	= sizeof(struct dm_test_priv),
};

U_BOOT_DRIVER(test2_drv) = {
	.name	= "test2_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_ops,
	.bind	= test_bind,
	.probe	= test_probe,
	.remove	= test_remove,
	.unbind	= test_unbind,
	.priv_auto	= sizeof(struct dm_test_priv),
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
	struct unit_test_state *uts = test_get_state();

	dm_testdrv_op_count[DM_TEST_OP_PROBE]++;
	if (!uts->force_fail_alloc)
		dev_set_priv(dev, calloc(1, sizeof(struct dm_test_priv)));
	if (!dev_get_priv(dev))
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

U_BOOT_DRIVER(test_vital_clk_drv) = {
	.name	= "test_vital_clk_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_manual_ops,
	.bind	= test_manual_bind,
	.probe	= test_manual_probe,
	.remove	= test_manual_remove,
	.unbind	= test_manual_unbind,
	.flags	= DM_FLAG_VITAL,
};

U_BOOT_DRIVER(test_act_dma_vital_clk_drv) = {
	.name	= "test_act_dma_vital_clk_drv",
	.id	= UCLASS_TEST,
	.ops	= &test_manual_ops,
	.bind	= test_manual_bind,
	.probe	= test_manual_probe,
	.remove	= test_manual_remove,
	.unbind	= test_manual_unbind,
	.flags	= DM_FLAG_VITAL | DM_FLAG_ACTIVE_DMA,
};

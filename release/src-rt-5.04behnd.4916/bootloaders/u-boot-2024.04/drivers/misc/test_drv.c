// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <asm/global_data.h>

/* Records the last testbus device that was removed */
static struct udevice *testbus_removed;

struct udevice *testbus_get_clear_removed(void)
{
	struct udevice *removed = testbus_removed;

	testbus_removed = NULL;

	return removed;
}

static int testbus_drv_probe(struct udevice *dev)
{
	if (!CONFIG_IS_ENABLED(OF_PLATDATA)) {
		int ret;

		ret = dm_scan_fdt_dev(dev);
		if (ret)
			return ret;
	}

	return 0;
}

static int testbus_child_post_bind(struct udevice *dev)
{
	struct dm_test_parent_plat *plat;

	plat = dev_get_parent_plat(dev);
	plat->bind_flag = 1;
	plat->uclass_bind_flag = 2;

	return 0;
}

static int testbus_child_pre_probe(struct udevice *dev)
{
	struct dm_test_parent_data *parent_data = dev_get_parent_priv(dev);

	parent_data->flag += TEST_FLAG_CHILD_PROBED;

	return 0;
}

static int testbus_child_pre_probe_uclass(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->uclass_flag++;

	return 0;
}

static int testbus_child_post_probe_uclass(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->uclass_postp++;

	return 0;
}

static int testbus_child_post_remove(struct udevice *dev)
{
	struct dm_test_parent_data *parent_data = dev_get_parent_priv(dev);

	parent_data->flag += TEST_FLAG_CHILD_REMOVED;
	testbus_removed = dev;

	return 0;
}

static const struct udevice_id testbus_ids[] = {
	{ .compatible = "denx,u-boot-test-bus", .data = DM_TEST_TYPE_FIRST },
	{ }
};

U_BOOT_DRIVER(denx_u_boot_test_bus) = {
	.name	= "testbus_drv",
	.of_match	= testbus_ids,
	.id	= UCLASS_TEST_BUS,
	.probe	= testbus_drv_probe,
	.child_post_bind = testbus_child_post_bind,
	.priv_auto	= sizeof(struct dm_test_priv),
	.plat_auto	= sizeof(struct dm_test_pdata),
	.per_child_auto	= sizeof(struct dm_test_parent_data),
	.per_child_plat_auto	= sizeof(struct dm_test_parent_plat),
	.child_pre_probe = testbus_child_pre_probe,
	.child_post_remove = testbus_child_post_remove,
	DM_HEADER(<test.h>)
};

UCLASS_DRIVER(testbus) = {
	.name		= "testbus",
	.id		= UCLASS_TEST_BUS,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.child_pre_probe = testbus_child_pre_probe_uclass,
	.child_post_probe = testbus_child_post_probe_uclass,

	.per_device_auto   = sizeof(struct dm_test_uclass_priv),

	/* Note: this is for dtoc testing as well as tags*/
	.per_device_plat_auto   = sizeof(struct dm_test_uclass_plat),
};

static int testfdt_drv_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct dm_test_pdata *pdata = dev_get_plat(dev);
	struct dm_test_priv *priv = dev_get_priv(dev);

	*pingret = pingval + pdata->ping_add;
	priv->ping_total += *pingret;

	return 0;
}

static const struct test_ops test_ops = {
	.ping = testfdt_drv_ping,
};

static int testfdt_of_to_plat(struct udevice *dev)
{
	struct dm_test_pdata *pdata = dev_get_plat(dev);

	pdata->ping_add = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					 "ping-add", -1);
	pdata->base = fdtdec_get_addr(gd->fdt_blob, dev_of_offset(dev),
				      "ping-expect");

	return 0;
}

static int testfdt_drv_probe(struct udevice *dev)
{
	struct dm_test_priv *priv = dev_get_priv(dev);

	priv->ping_total += DM_TEST_START_TOTAL;

	/*
	 * If this device is on a bus, the uclass_flag will be set before
	 * calling this function. In the meantime the uclass_postp is
	 * initlized to a value -1. These are used respectively by
	 * dm_test_bus_child_pre_probe_uclass() and
	 * dm_test_bus_child_post_probe_uclass().
	 */
	priv->uclass_total += priv->uclass_flag;
	priv->uclass_postp = -1;

	return 0;
}

static const struct udevice_id testfdt_ids[] = {
	{ .compatible = "denx,u-boot-fdt-test", .data = DM_TEST_TYPE_FIRST },
	{ .compatible = "google,another-fdt-test", .data = DM_TEST_TYPE_SECOND },
	{ }
};

DM_DRIVER_ALIAS(denx_u_boot_fdt_test, google_another_fdt_test)

U_BOOT_DRIVER(denx_u_boot_fdt_test) = {
	.name	= "testfdt_drv",
	.of_match	= testfdt_ids,
	.id	= UCLASS_TEST_FDT,
	.of_to_plat = testfdt_of_to_plat,
	.probe	= testfdt_drv_probe,
	.ops	= &test_ops,
	.priv_auto	= sizeof(struct dm_test_priv),
	.plat_auto	= sizeof(struct dm_test_pdata),
};

static const struct udevice_id testfdt1_ids[] = {
	{ .compatible = "denx,u-boot-fdt-test1", .data = DM_TEST_TYPE_FIRST },
	{ }
};

U_BOOT_DRIVER(testfdt1_drv) = {
	.name	= "testfdt1_drv",
	.of_match	= testfdt1_ids,
	.id	= UCLASS_TEST_FDT,
	.of_to_plat = testfdt_of_to_plat,
	.probe	= testfdt_drv_probe,
	.ops	= &test_ops,
	.priv_auto	= sizeof(struct dm_test_priv),
	.plat_auto	= sizeof(struct dm_test_pdata),
	.flags = DM_FLAG_PRE_RELOC,
};

/* From here is the testfdt uclass code */
int testfdt_ping(struct udevice *dev, int pingval, int *pingret)
{
	const struct test_ops *ops = device_get_ops(dev);

	if (!ops->ping)
		return -ENOSYS;

	return ops->ping(dev, pingval, pingret);
}

UCLASS_DRIVER(testfdt) = {
	.name		= "testfdt",
	.id		= UCLASS_TEST_FDT,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto	= sizeof(struct dm_test_uc_priv),
};

static const struct udevice_id testfdtm_ids[] = {
	{ .compatible = "denx,u-boot-fdtm-test" },
	{ }
};

U_BOOT_DRIVER(testfdtm_drv) = {
	.name	= "testfdtm_drv",
	.of_match	= testfdtm_ids,
	.id	= UCLASS_TEST_FDT_MANUAL,
};

UCLASS_DRIVER(testfdtm) = {
	.name		= "testfdtm",
	.id		= UCLASS_TEST_FDT_MANUAL,
	.flags		= DM_UC_FLAG_SEQ_ALIAS | DM_UC_FLAG_NO_AUTO_SEQ,
};

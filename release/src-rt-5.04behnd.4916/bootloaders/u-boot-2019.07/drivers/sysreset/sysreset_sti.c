// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <sysreset.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct sti_sysreset_priv {
	phys_addr_t base;
};

static int sti_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct sti_sysreset_priv *priv = dev_get_priv(dev);

	generic_clear_bit(0, (void __iomem *)priv->base);

	return -EINPROGRESS;
}

static int sti_sysreset_probe(struct udevice *dev)
{
	struct sti_sysreset_priv *priv = dev_get_priv(dev);
	struct udevice *syscon;
	struct regmap *regmap;
	struct fdtdec_phandle_args syscfg_phandle;
	int ret;

	/* get corresponding syscon phandle */
	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev_of_offset(dev),
					     "st,syscfg", NULL, 0, 0,
					     &syscfg_phandle);
	if (ret < 0) {
		pr_err("Can't get syscfg phandle: %d\n", ret);
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_SYSCON,
					     syscfg_phandle.node,
					     &syscon);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset failed: %d\n",
		      __func__, ret);
		return ret;
	}

	regmap = syscon_get_regmap(syscon);
	if (!regmap) {
		pr_err("unable to get regmap for %s\n", syscon->name);
		return -ENODEV;
	}

	priv->base = regmap->ranges[0].start;

	return 0;
}

static struct sysreset_ops sti_sysreset = {
	.request	= sti_sysreset_request,
};

static const struct udevice_id sti_sysreset_ids[] = {
	{ .compatible = "st,stih407-restart" },
	{ }
};

U_BOOT_DRIVER(sysreset_sti) = {
	.name = "sysreset_sti",
	.id = UCLASS_SYSRESET,
	.ops = &sti_sysreset,
	.probe = sti_sysreset_probe,
	.of_match = sti_sysreset_ids,
	.priv_auto_alloc_size = sizeof(struct sti_sysreset_priv),
};

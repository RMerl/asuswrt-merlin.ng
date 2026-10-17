// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/types.h>
#include <cpu.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <linux/err.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct mtk_cpu_plat {
	struct regmap *hwver;
};

static int mtk_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct mtk_cpu_plat *plat = dev_get_plat(dev);
	uint val;

	regmap_read(plat->hwver, 0, &val);

	snprintf(buf, size, "MediaTek MT%04X", val);

	return 0;
}

static int mtk_cpu_get_count(const struct udevice *dev)
{
	return 1;
}

static int mtk_cpu_get_vendor(const struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "MediaTek");

	return 0;
}

static int mtk_cpu_probe(struct udevice *dev)
{
	struct mtk_cpu_plat *plat = dev_get_plat(dev);
	struct ofnode_phandle_args args;
	int ret;

	ret = dev_read_phandle_with_args(dev, "mediatek,hwver", NULL, 0, 0,
					 &args);
	if (ret)
		return ret;

	plat->hwver = syscon_node_to_regmap(args.node);
	if (IS_ERR(plat->hwver))
		return PTR_ERR(plat->hwver);

	return 0;
}

static const struct cpu_ops mtk_cpu_ops = {
	.get_desc	= mtk_cpu_get_desc,
	.get_count	= mtk_cpu_get_count,
	.get_vendor	= mtk_cpu_get_vendor,
};

static const struct udevice_id mtk_cpu_ids[] = {
	{ .compatible = "arm,cortex-a7" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a73" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(cpu_mtk) = {
	.name		= "mtk-cpu",
	.id		= UCLASS_CPU,
	.of_match	= mtk_cpu_ids,
	.ops		= &mtk_cpu_ops,
	.probe		= mtk_cpu_probe,
	.plat_auto	= sizeof(struct mtk_cpu_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * UniPhier Specific Glue Layer for DWC3
 *
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *   Author: Kunihiko Hayashi <hayashi.kunihiko@socionext.com>
 */

#include <dm.h>
#include <dm/lists.h>
#include <linux/bitops.h>
#include <linux/usb/gadget.h>

#include "core.h"
#include "gadget.h"
#include "dwc3-generic.h"

#define UNIPHIER_PRO4_DWC3_RESET	0x40
#define   UNIPHIER_PRO4_DWC3_RESET_XIOMMU	BIT(5)
#define   UNIPHIER_PRO4_DWC3_RESET_XLINK	BIT(4)
#define   UNIPHIER_PRO4_DWC3_RESET_PHY_SS	BIT(2)

#define UNIPHIER_PRO5_DWC3_RESET	0x00
#define   UNIPHIER_PRO5_DWC3_RESET_PHY_S1	BIT(17)
#define   UNIPHIER_PRO5_DWC3_RESET_PHY_S0	BIT(16)
#define   UNIPHIER_PRO5_DWC3_RESET_XLINK	BIT(15)
#define   UNIPHIER_PRO5_DWC3_RESET_XIOMMU	BIT(14)

#define UNIPHIER_PXS2_DWC3_RESET	0x00
#define   UNIPHIER_PXS2_DWC3_RESET_XLINK	BIT(15)

static void uniphier_pro4_dwc3_init(struct udevice *dev, int index,
				    enum usb_dr_mode mode)
{
	struct dwc3_glue_data *glue = dev_get_plat(dev);
	void *regs = map_physmem(glue->regs, glue->size, MAP_NOCACHE);
	u32 tmp;

	tmp = readl(regs + UNIPHIER_PRO4_DWC3_RESET);
	tmp &= ~UNIPHIER_PRO4_DWC3_RESET_PHY_SS;
	tmp |= UNIPHIER_PRO4_DWC3_RESET_XIOMMU | UNIPHIER_PRO4_DWC3_RESET_XLINK;
	writel(tmp, regs + UNIPHIER_PRO4_DWC3_RESET);

	unmap_physmem(regs, MAP_NOCACHE);
}

static void uniphier_pro5_dwc3_init(struct udevice *dev, int index,
				    enum usb_dr_mode mode)
{
	struct dwc3_glue_data *glue = dev_get_plat(dev);
	void *regs = map_physmem(glue->regs, glue->size, MAP_NOCACHE);
	u32 tmp;

	tmp = readl(regs + UNIPHIER_PRO5_DWC3_RESET);
	tmp &= ~(UNIPHIER_PRO5_DWC3_RESET_PHY_S1 |
		 UNIPHIER_PRO5_DWC3_RESET_PHY_S0);
	tmp |= UNIPHIER_PRO5_DWC3_RESET_XLINK | UNIPHIER_PRO5_DWC3_RESET_XIOMMU;
	writel(tmp, regs + UNIPHIER_PRO5_DWC3_RESET);

	unmap_physmem(regs, MAP_NOCACHE);
}

static void uniphier_pxs2_dwc3_init(struct udevice *dev, int index,
				    enum usb_dr_mode mode)
{
	struct dwc3_glue_data *glue = dev_get_plat(dev);
	void *regs = map_physmem(glue->regs, glue->size, MAP_NOCACHE);
	u32 tmp;

	tmp = readl(regs + UNIPHIER_PXS2_DWC3_RESET);
	tmp |= UNIPHIER_PXS2_DWC3_RESET_XLINK;
	writel(tmp, regs + UNIPHIER_PXS2_DWC3_RESET);

	unmap_physmem(regs, MAP_NOCACHE);
}

static int dwc3_uniphier_glue_get_ctrl_dev(struct udevice *dev, ofnode *node)
{
	struct udevice *child;
	const char *name;
	ofnode subnode;

	/*
	 * "controller reset" belongs to glue logic, and it should be
	 * accessible in .glue_configure() before access to the controller
	 * begins.
	 */
	ofnode_for_each_subnode(subnode, dev_ofnode(dev)) {
		name = ofnode_get_name(subnode);
		if (!strncmp(name, "reset", 5))
			device_bind_driver_to_node(dev, "uniphier-reset",
						   name, subnode, &child);
	}

	/* Get controller node that is placed separately from the glue node */
	*node = ofnode_by_compatible(dev_ofnode(dev->parent),
				     "socionext,uniphier-dwc3");

	return 0;
}

static const struct dwc3_glue_ops uniphier_pro4_dwc3_ops = {
	.glue_get_ctrl_dev = dwc3_uniphier_glue_get_ctrl_dev,
	.glue_configure = uniphier_pro4_dwc3_init,
};

static const struct dwc3_glue_ops uniphier_pro5_dwc3_ops = {
	.glue_get_ctrl_dev = dwc3_uniphier_glue_get_ctrl_dev,
	.glue_configure = uniphier_pro5_dwc3_init,
};

static const struct dwc3_glue_ops uniphier_pxs2_dwc3_ops = {
	.glue_get_ctrl_dev = dwc3_uniphier_glue_get_ctrl_dev,
	.glue_configure = uniphier_pxs2_dwc3_init,
};

static const struct udevice_id uniphier_dwc3_match[] = {
	{
		.compatible = "socionext,uniphier-pro4-dwc3-glue",
		.data = (ulong)&uniphier_pro4_dwc3_ops,
	},
	{
		.compatible = "socionext,uniphier-pro5-dwc3-glue",
		.data = (ulong)&uniphier_pro5_dwc3_ops,
	},
	{
		.compatible = "socionext,uniphier-pxs2-dwc3-glue",
		.data = (ulong)&uniphier_pxs2_dwc3_ops,
	},
	{
		.compatible = "socionext,uniphier-ld20-dwc3-glue",
		.data = (ulong)&uniphier_pxs2_dwc3_ops,
	},
	{
		.compatible = "socionext,uniphier-pxs3-dwc3-glue",
		.data = (ulong)&uniphier_pxs2_dwc3_ops,
	},
	{
		.compatible = "socionext,uniphier-nx1-dwc3-glue",
		.data = (ulong)&uniphier_pxs2_dwc3_ops,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(dwc3_uniphier_wrapper) = {
	.name = "uniphier-dwc3",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = uniphier_dwc3_match,
	.bind = dwc3_glue_bind,
	.probe = dwc3_glue_probe,
	.remove = dwc3_glue_remove,
	.plat_auto = sizeof(struct dwc3_glue_data),
};

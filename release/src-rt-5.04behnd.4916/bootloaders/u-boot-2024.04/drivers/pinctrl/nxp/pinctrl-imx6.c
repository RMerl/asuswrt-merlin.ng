// SPDX-License-Identifier: GPL-2.0+

/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx6_pinctrl_soc_info __section(".data");

/* FIXME Before reloaction, BSS is overlapped with DT area */
static struct imx_pinctrl_soc_info imx6ul_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
};

static struct imx_pinctrl_soc_info imx6_snvs_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
};

static int imx6_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id imx6_pinctrl_match[] = {
	{ .compatible = "fsl,imx6q-iomuxc", .data = (ulong)&imx6_pinctrl_soc_info },
	{ .compatible = "fsl,imx6dl-iomuxc", .data = (ulong)&imx6_pinctrl_soc_info },
	{ .compatible = "fsl,imx6sl-iomuxc", .data = (ulong)&imx6_pinctrl_soc_info },
	{ .compatible = "fsl,imx6sll-iomuxc-snvs", .data = (ulong)&imx6_snvs_pinctrl_soc_info },
	{ .compatible = "fsl,imx6sll-iomuxc", .data = (ulong)&imx6_pinctrl_soc_info },
	{ .compatible = "fsl,imx6sx-iomuxc", .data = (ulong)&imx6_pinctrl_soc_info },
	{ .compatible = "fsl,imx6ul-iomuxc", .data = (ulong)&imx6ul_pinctrl_soc_info },
	{ .compatible = "fsl,imx6ull-iomuxc-snvs", .data = (ulong)&imx6_snvs_pinctrl_soc_info },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(fsl_imx6q_iomuxc) = {
	.name = "fsl_imx6q_iomuxc",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx6_pinctrl_match),
	.probe = imx6_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

DM_DRIVER_ALIAS(fsl_imx6q_iomuxc, fsl_imx6dl_iomuxc)

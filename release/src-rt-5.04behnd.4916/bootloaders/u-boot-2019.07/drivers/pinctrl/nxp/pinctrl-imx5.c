// SPDX-License-Identifier: GPL-2.0+

/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx5_pinctrl_soc_info;

static int imx5_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id imx5_pinctrl_match[] = {
	{
		.compatible = "fsl,imx53-iomuxc",
		.data = (ulong)&imx5_pinctrl_soc_info
	},
	{
		.compatible = "fsl,imx53-iomuxc-gpr",
		.data = (ulong)&imx5_pinctrl_soc_info
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(imx5_pinctrl) = {
	.name = "imx5-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx5_pinctrl_match),
	.probe = imx5_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

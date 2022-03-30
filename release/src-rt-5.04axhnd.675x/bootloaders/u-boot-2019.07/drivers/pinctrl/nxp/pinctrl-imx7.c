// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Peng Fan <van.freenix@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx7_pinctrl_soc_info;

static struct imx_pinctrl_soc_info imx7_lpsr_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
};

static int imx7_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id imx7_pinctrl_match[] = {
	{ .compatible = "fsl,imx7d-iomuxc", .data = (ulong)&imx7_pinctrl_soc_info },
	{ .compatible = "fsl,imx7d-iomuxc-lpsr", .data = (ulong)&imx7_lpsr_pinctrl_soc_info },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(imx7_pinctrl) = {
	.name = "imx7-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx7_pinctrl_match),
	.probe = imx7_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

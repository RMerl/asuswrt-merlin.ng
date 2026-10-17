// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imxrt_pinctrl_soc_info = {
	.flags = ZERO_OFFSET_VALID,
};

static int imxrt_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id imxrt_pinctrl_match[] = {
	{ .compatible = "fsl,imxrt-iomuxc",
	  .data = (ulong)&imxrt_pinctrl_soc_info },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(imxrt_pinctrl) = {
	.name = "imxrt-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imxrt_pinctrl_match),
	.probe = imxrt_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <dm/device.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info imx8mq_pinctrl_soc_info __section(".data");

static int imx8mq_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id imx8m_pinctrl_match[] = {
	{ .compatible = "fsl,imx8mq-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mm-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mn-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ .compatible = "fsl,imx8mp-iomuxc", .data = (ulong)&imx8mq_pinctrl_soc_info },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(imx8mq_pinctrl) = {
	.name = "imx8mq-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(imx8m_pinctrl_match),
	.probe = imx8mq_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto	= sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

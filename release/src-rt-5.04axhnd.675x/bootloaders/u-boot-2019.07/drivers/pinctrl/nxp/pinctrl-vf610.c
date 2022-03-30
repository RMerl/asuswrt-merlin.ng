// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>

#include "pinctrl-imx.h"

static struct imx_pinctrl_soc_info vf610_pinctrl_soc_info = {
	.flags = SHARE_MUX_CONF_REG | ZERO_OFFSET_VALID,
};

static int vf610_pinctrl_probe(struct udevice *dev)
{
	struct imx_pinctrl_soc_info *info =
		(struct imx_pinctrl_soc_info *)dev_get_driver_data(dev);

	return imx_pinctrl_probe(dev, info);
}

static const struct udevice_id vf610_pinctrl_match[] = {
	{ .compatible = "fsl,vf610-iomuxc",
	  .data = (ulong)&vf610_pinctrl_soc_info },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(vf610_pinctrl) = {
	.name = "vf610-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(vf610_pinctrl_match),
	.probe = vf610_pinctrl_probe,
	.remove = imx_pinctrl_remove,
	.priv_auto_alloc_size = sizeof(struct imx_pinctrl_priv),
	.ops = &imx_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

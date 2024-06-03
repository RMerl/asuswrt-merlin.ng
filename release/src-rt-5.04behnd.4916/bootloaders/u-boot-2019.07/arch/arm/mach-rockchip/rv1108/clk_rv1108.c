// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rv1108.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_GET_DRIVER(clk_rv1108), devp);
}

void *rockchip_get_cru(void)
{
	struct rv1108_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->cru;
}

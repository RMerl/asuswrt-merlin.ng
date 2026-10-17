// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru.h>
#include <linux/err.h>

static int rockchip_get_cruclk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(clk_rk3399), devp);
}

void *rockchip_get_cru(void)
{
	struct rk3399_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_cruclk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->cru;
}

static int rockchip_get_pmucruclk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3399_pmuclk), devp);
}

void *rockchip_get_pmucru(void)
{
	struct rk3399_pmuclk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_pmucruclk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->pmucru;
}

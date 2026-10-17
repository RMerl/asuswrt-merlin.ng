// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3568.h>
#include <linux/err.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3568_cru), devp);
}

void *rockchip_get_cru(void)
{
	struct rk3568_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->cru;
}

static int rockchip_get_pmucruclk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3568_pmucru), devp);
}

void *rockchip_get_pmucru(void)
{
	struct rk3568_pmuclk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_pmucruclk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->pmucru;
}

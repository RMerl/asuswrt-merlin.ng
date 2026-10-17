// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/arch/grf_rk3308.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/sdram.h>

struct dram_info {
	struct ram_info info;
	struct rk3308_grf *grf;
};

static int rk3308_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->info.base = CFG_SYS_SDRAM_BASE;
	priv->info.size = rockchip_sdram_size((phys_addr_t)&priv->grf->os_reg2);

	return 0;
}

static int rk3308_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3308_dmc_ops = {
	.get_info = rk3308_dmc_get_info,
};

static const struct udevice_id rk3308_dmc_ids[] = {
	{ .compatible = "rockchip,rk3308-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3308) = {
	.name = "rockchip_rk3308_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3308_dmc_ids,
	.ops = &rk3308_dmc_ops,
	.probe = rk3308_dmc_probe,
	.priv_auto	= sizeof(struct dram_info),
};

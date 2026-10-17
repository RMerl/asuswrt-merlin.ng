// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3568.h>
#include <asm/arch-rockchip/sdram.h>

struct dram_info {
	struct ram_info info;
	struct rk3568_pmugrf *pmugrf;
};

static int rk3568_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);
	priv->info.base = CFG_SYS_SDRAM_BASE;
	priv->info.size =
		rockchip_sdram_size((phys_addr_t)&priv->pmugrf->pmu_os_reg2);

	return 0;
}

static int rk3568_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3568_dmc_ops = {
	.get_info = rk3568_dmc_get_info,
};

static const struct udevice_id rk3568_dmc_ids[] = {
	{ .compatible = "rockchip,rk3568-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3568) = {
	.name = "rockchip_rk3568_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3568_dmc_ids,
	.ops = &rk3568_dmc_ops,
	.probe = rk3568_dmc_probe,
	.priv_auto = sizeof(struct dram_info),
};

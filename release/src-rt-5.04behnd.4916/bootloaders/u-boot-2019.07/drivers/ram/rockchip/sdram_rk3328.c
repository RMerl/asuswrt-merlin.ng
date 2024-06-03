// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3328.h>
#include <asm/arch-rockchip/sdram_common.h>

struct dram_info {
	struct ram_info info;
	struct rk3328_grf_regs *grf;
};

static int rk3328_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	debug("%s: grf=%p\n", __func__, priv->grf);
	priv->info.base = CONFIG_SYS_SDRAM_BASE;
	priv->info.size = rockchip_sdram_size(
				(phys_addr_t)&priv->grf->os_reg[2]);

	return 0;
}

static int rk3328_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3328_dmc_ops = {
	.get_info = rk3328_dmc_get_info,
};


static const struct udevice_id rk3328_dmc_ids[] = {
	{ .compatible = "rockchip,rk3328-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3328) = {
	.name = "rockchip_rk3328_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3328_dmc_ids,
	.ops = &rk3328_dmc_ops,
	.probe = rk3328_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
};

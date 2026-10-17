// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3399_syscon_ids[] = {
	{ .compatible = "rockchip,rk3399-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3399-pmugrf", .data = ROCKCHIP_SYSCON_PMUGRF },
	{ .compatible = "rockchip,rk3399-pmusgrf", .data = ROCKCHIP_SYSCON_PMUSGRF },
	{ .compatible = "rockchip,rk3399-cic", .data = ROCKCHIP_SYSCON_CIC },
	{ .compatible = "rockchip,rk3399-pmu", .data = ROCKCHIP_SYSCON_PMU },
	{ }
};

U_BOOT_DRIVER(syscon_rk3399) = {
	.name = "rk3399_syscon",
	.id = UCLASS_SYSCON,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
	.of_match = rk3399_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int rk3399_syscon_bind_of_plat(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}

U_BOOT_DRIVER(rockchip_rk3399_grf) = {
	.name = "rockchip_rk3399_grf",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids,
	.bind = rk3399_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_rk3399_pmugrf) = {
	.name = "rockchip_rk3399_pmugrf",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids + 1,
	.bind = rk3399_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_rk3399_pmusgrf) = {
	.name = "rockchip_rk3399_pmusgrf",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids + 2,
	.bind = rk3399_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_rk3399_cic) = {
	.name = "rockchip_rk3399_cic",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids + 3,
	.bind = rk3399_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_rk3399_pmu) = {
	.name = "rockchip_rk3399_pmu",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids + 4,
	.bind = rk3399_syscon_bind_of_plat,
};
#endif

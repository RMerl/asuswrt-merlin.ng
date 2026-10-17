// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id px30_syscon_ids[] = {
	{ .compatible = "rockchip,px30-pmu", .data = ROCKCHIP_SYSCON_PMU },
	{ .compatible = "rockchip,px30-pmugrf", .data = ROCKCHIP_SYSCON_PMUGRF },
	{ .compatible = "rockchip,px30-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(syscon_px30) = {
	.id = UCLASS_SYSCON,
	.name = "px30_syscon",
	.of_match = px30_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int px30_syscon_bind_of_plat(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}

U_BOOT_DRIVER(rockchip_px30_pmu) = {
	.name = "rockchip_px30_pmu",
	.id = UCLASS_SYSCON,
	.of_match = px30_syscon_ids,
	.bind = px30_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_px30_pmugrf) = {
	.name = "rockchip_px30_pmugrf",
	.id = UCLASS_SYSCON,
	.of_match = px30_syscon_ids + 1,
	.bind = px30_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_px30_grf) = {
	.name = "rockchip_px30_grf",
	.id = UCLASS_SYSCON,
	.of_match = px30_syscon_ids + 2,
	.bind = px30_syscon_bind_of_plat,
};
#endif

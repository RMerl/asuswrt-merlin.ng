// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3368_syscon_ids[] = {
	{ .compatible = "rockchip,rk3368-grf",
	  .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3368-pmugrf",
	  .data = ROCKCHIP_SYSCON_PMUGRF },
	{ .compatible = "rockchip,rk3368-msch",
	  .data = ROCKCHIP_SYSCON_MSCH },
	{ .compatible = "rockchip,rk3368-sgrf",
	  .data = ROCKCHIP_SYSCON_SGRF },
	{ }
};

U_BOOT_DRIVER(syscon_rk3368) = {
	.name = "rk3368_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3368_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int rk3368_syscon_bind_of_platdata(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}

U_BOOT_DRIVER(rockchip_rk3368_grf) = {
	.name = "rockchip_rk3368_grf",
	.id = UCLASS_SYSCON,
	.of_match = rk3368_syscon_ids,
	.bind = rk3368_syscon_bind_of_platdata,
};

U_BOOT_DRIVER(rockchip_rk3368_pmugrf) = {
	.name = "rockchip_rk3368_pmugrf",
	.id = UCLASS_SYSCON,
	.of_match = rk3368_syscon_ids + 1,
	.bind = rk3368_syscon_bind_of_platdata,
};

U_BOOT_DRIVER(rockchip_rk3368_msch) = {
	.name = "rockchip_rk3368_msch",
	.id = UCLASS_SYSCON,
	.of_match = rk3368_syscon_ids + 2,
	.bind = rk3368_syscon_bind_of_platdata,
};

U_BOOT_DRIVER(rockchip_rk3368_sgrf) = {
	.name = "rockchip_rk3368_sgrf",
	.id = UCLASS_SYSCON,
	.of_match = rk3368_syscon_ids + 3,
	.bind = rk3368_syscon_bind_of_platdata,
};
#endif

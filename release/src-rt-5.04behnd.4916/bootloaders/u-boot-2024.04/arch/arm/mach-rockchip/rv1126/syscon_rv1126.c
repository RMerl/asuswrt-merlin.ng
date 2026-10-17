// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rv1126_syscon_ids[] = {
	{ .compatible = "rockchip,rv1126-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rv1126-pmugrf", .data = ROCKCHIP_SYSCON_PMUGRF },
	{ }
};

U_BOOT_DRIVER(syscon_rv1126) = {
	.name = "rv1126_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rv1126_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int rv1126_syscon_bind_of_plat(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}

U_BOOT_DRIVER(rockchip_rv1126_pmu) = {
	.name = "rockchip_rv1126_pmu",
	.id = UCLASS_SYSCON,
	.of_match = rv1126_syscon_ids,
	.bind = rv1126_syscon_bind_of_plat,
};

U_BOOT_DRIVER(rockchip_rv1126_pmugrf) = {
	.name = "rockchip_rv1126_pmugrf",
	.id = UCLASS_SYSCON,
	.of_match = rv1126_syscon_ids + 1,
	.bind = rv1126_syscon_bind_of_plat,
};
#endif

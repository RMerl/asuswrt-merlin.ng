// SPDX-License-Identifier:     GPL-2.0+
/*
 * (C) Copyright 2018 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3308_syscon_ids[] = {
	{ .compatible = "rockchip,rk3308-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(syscon_rk3308) = {
	.name = "rk3308_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3308_syscon_ids,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3128_syscon_ids[] = {
	{ .compatible = "rockchip,rk3128-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(syscon_rk3128) = {
	.name = "rk3128_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3128_syscon_ids,
};

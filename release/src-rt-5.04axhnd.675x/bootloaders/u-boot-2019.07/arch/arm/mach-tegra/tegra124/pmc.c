// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>

static const struct udevice_id tegra124_syscon_ids[] = {
	{ .compatible = "nvidia,tegra124-pmc", .data = TEGRA_SYSCON_PMC },
};

U_BOOT_DRIVER(syscon_tegra124) = {
	.name = "tegra124_syscon",
	.id = UCLASS_SYSCON,
	.of_match = tegra124_syscon_ids,
};

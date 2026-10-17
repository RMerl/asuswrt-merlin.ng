// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <dm.h>

static const struct udevice_id sandbox_memory_match[] = {
	{ .compatible = "sandbox,memory" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_memory) = {
	.name	= "sandbox_memory",
	.id	= UCLASS_MEMORY,
	.of_match = sandbox_memory_match,
};

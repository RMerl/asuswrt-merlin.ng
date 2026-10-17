// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <dm.h>

UCLASS_DRIVER(memory) = {
	.name = "memory",
	.id = UCLASS_MEMORY,
	.post_bind = dm_scan_fdt_dev,
};

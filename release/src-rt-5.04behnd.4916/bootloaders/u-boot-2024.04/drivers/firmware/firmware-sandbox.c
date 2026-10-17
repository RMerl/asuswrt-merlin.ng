// SPDX-License-Identifier: GPL-2.0+
/*
 * sandbox firmware driver
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>

static const struct udevice_id generic_sandbox_firmware_ids[] = {
	{ .compatible = "sandbox,firmware" },
	{ }
};

U_BOOT_DRIVER(sandbox_firmware) = {
	.name = "sandbox_firmware",
	.id = UCLASS_FIRMWARE,
	.of_match = generic_sandbox_firmware_ids,
};

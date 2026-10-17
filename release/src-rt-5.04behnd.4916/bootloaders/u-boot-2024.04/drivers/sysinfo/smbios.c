// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <sysinfo.h>

static const struct udevice_id sysinfo_smbios_ids[] = {
	{ .compatible = "u-boot,sysinfo-smbios" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_smbios_ops = {
};

U_BOOT_DRIVER(sysinfo_smbios) = {
	.name           = "sysinfo_smbios",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_smbios_ids,
	.ops		= &sysinfo_smbios_ops,
};

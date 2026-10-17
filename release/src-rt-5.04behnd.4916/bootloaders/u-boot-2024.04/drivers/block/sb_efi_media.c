// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI_MEDIA driver for sandbox
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <dm.h>

static const struct udevice_id sandbox_efi_media_ids[] = {
	{ .compatible = "sandbox,efi-media" },
	{ }
};

U_BOOT_DRIVER(sandbox_efi_media) = {
	.name		= "sandbox_efi_media",
	.id		= UCLASS_EFI_MEDIA,
	.of_match	= sandbox_efi_media_ids,
};

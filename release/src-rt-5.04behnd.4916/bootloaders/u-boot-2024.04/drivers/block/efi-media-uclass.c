// SPDX-License-Identifier: GPL-2.0+
/*
 * Uclass for EFI media devices
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <dm.h>

UCLASS_DRIVER(efi_media) = {
	.id		= UCLASS_EFI_MEDIA,
	.name		= "efi_media",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>

UCLASS_DRIVER(ahci) = {
	.id		= UCLASS_AHCI,
	.name		= "ahci",
	.per_device_auto_alloc_size = sizeof(struct ahci_uc_priv),
};

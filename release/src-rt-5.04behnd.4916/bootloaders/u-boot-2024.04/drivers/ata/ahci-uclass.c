// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_AHCI

#include <common.h>
#include <ahci.h>
#include <dm.h>

UCLASS_DRIVER(ahci) = {
	.id		= UCLASS_AHCI,
	.name		= "ahci",
	.per_device_auto	= sizeof(struct ahci_uc_priv),
};

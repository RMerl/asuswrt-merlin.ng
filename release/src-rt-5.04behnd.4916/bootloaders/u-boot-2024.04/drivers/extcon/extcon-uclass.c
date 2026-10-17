// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY UCLASS_EXTCON

#include <common.h>
#include <extcon.h>
#include <dm.h>

UCLASS_DRIVER(extcon) = {
	.id			= UCLASS_EXTCON,
	.name			= "extcon",
	.per_device_plat_auto	= sizeof(struct extcon_uc_plat),
};

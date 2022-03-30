// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/root.h>

UCLASS_DRIVER(northbridge) = {
	.id		= UCLASS_NORTHBRIDGE,
	.name		= "northbridge",
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>

static const struct udevice_id sandbox_spl_ids[] = {
	{ .compatible = "sandbox,spl-test", },
	{}  /* sentinel */
};

U_BOOT_DRIVER(sandbox_spl_test) = {
	.name	= "sandbox_spl_test",
	.id	= UCLASS_MISC,
	.of_match = sandbox_spl_ids,
	.flags	= DM_FLAG_PRE_RELOC,
};

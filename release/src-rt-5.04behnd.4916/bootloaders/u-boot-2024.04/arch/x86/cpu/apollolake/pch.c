// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <pch.h>
#include <spl.h>
#include <asm/lpc_common.h>

#define BIOS_CTRL	0xdc

static int apl_set_spi_protect(struct udevice *dev, bool protect)
{
	if (spl_phase() == PHASE_SPL)
		return lpc_set_spi_protect(dev, BIOS_CTRL, protect);

	return 0;
}

static const struct pch_ops apl_pch_ops = {
	.set_spi_protect = apl_set_spi_protect,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id apl_pch_ids[] = {
	{ .compatible = "intel,apl-pch" },
	{ }
};
#endif

U_BOOT_DRIVER(intel_apl_pch) = {
	.name		= "intel_apl_pch",
	.id		= UCLASS_PCH,
	.of_match	= of_match_ptr(apl_pch_ids),
	.ops		= &apl_pch_ops,
};

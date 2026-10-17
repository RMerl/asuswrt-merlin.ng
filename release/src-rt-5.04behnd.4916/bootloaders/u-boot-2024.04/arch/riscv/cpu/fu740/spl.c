// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020-2021 SiFive, Inc
 * Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <dm.h>
#include <log.h>
#include <asm/csr.h>

#define CSR_U74_FEATURE_DISABLE	0x7c1

int spl_soc_init(void)
{
	int ret;
	struct udevice *dev;

	/* DDR init */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}

void harts_early_init(void)
{
	/*
	 * Feature Disable CSR
	 *
	 * Clear feature disable CSR to '0' to turn on all features for
	 * each core. This operation must be in M-mode.
	 */
	if (CONFIG_IS_ENABLED(RISCV_MMODE))
		csr_write(CSR_U74_FEATURE_DISABLE, 0);
}

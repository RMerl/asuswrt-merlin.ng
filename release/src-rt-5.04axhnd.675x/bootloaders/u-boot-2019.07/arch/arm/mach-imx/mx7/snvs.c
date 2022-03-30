// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Linaro
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <linux/bitops.h>

#define SNVS_HPCOMR		0x04
#define SNVS_HPCOMR_NPSWA_EN	BIT(31)

void init_snvs(void)
{
	u32 val;

	/* Ensure SNVS HPCOMR sets NPSWA_EN to allow unpriv access to SNVS LP */
	val = readl(SNVS_BASE_ADDR + SNVS_HPCOMR);
	val |= SNVS_HPCOMR_NPSWA_EN;
	writel(val, SNVS_BASE_ADDR + SNVS_HPCOMR);
}

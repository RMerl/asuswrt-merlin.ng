// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include <asm/arch/rst.h>

void reset_cpu(void)
{
	/* Generate a watchdog0 reset */
	writel(WTCR_WTR | WTCR_WTRE | WTCR_WTE, WTCR0_REG);

	while (1)
		;
}

void reset_misc(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	clrbits_le32(&gcr->intcr2, INTCR2_WDC);
}

int npcm_get_reset_status(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;
	u32 val;

	val = readl(&gcr->ressr);
	if (!val)
		val = readl(&gcr->intcr2);

	return val & RST_STS_MASK;
}

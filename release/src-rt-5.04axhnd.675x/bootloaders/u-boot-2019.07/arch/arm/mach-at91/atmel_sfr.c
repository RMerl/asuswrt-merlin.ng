// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/sama5_sfr.h>

void redirect_int_from_saic_to_aic(void)
{
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;
	u32 key32;

	if (!(readl(&sfr->aicredir) & ATMEL_SFR_AICREDIR_NSAIC)) {
		key32 = readl(&sfr->sn1) ^ ATMEL_SFR_AICREDIR_KEY;
		writel((key32 | ATMEL_SFR_AICREDIR_NSAIC), &sfr->aicredir);
	}
}

void configure_2nd_sram_as_l2_cache(void)
{
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;

	writel(1, &sfr->l2cc_hramc);
}

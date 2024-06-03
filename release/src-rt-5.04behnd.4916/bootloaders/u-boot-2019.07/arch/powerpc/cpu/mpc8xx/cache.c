// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Christophe Leroy, CS Systemes d'Information, christophe.leroy@c-s.fr
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/ppc.h>
#include <asm/io.h>
#include <asm/mmu.h>

int icache_status(void)
{
	return !!(mfspr(IC_CST) & IDC_ENABLED);
}

void icache_enable(void)
{
	sync();
	mtspr(IC_CST, IDC_INVALL);
	mtspr(IC_CST, IDC_ENABLE);
}

void icache_disable(void)
{
	sync();
	mtspr(IC_CST, IDC_DISABLE);
}

int dcache_status(void)
{
	return !!(mfspr(IC_CST) & IDC_ENABLED);
}

void dcache_enable(void)
{
	mtspr(MD_CTR, MD_RESETVAL);	/* Set cache mode with MMU off */
	mtspr(DC_CST, IDC_INVALL);
	mtspr(DC_CST, IDC_ENABLE);
}

void dcache_disable(void)
{
	sync();
	mtspr(DC_CST, IDC_DISABLE);
	mtspr(DC_CST, IDC_INVALL);
}

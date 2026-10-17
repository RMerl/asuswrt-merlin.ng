// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_rstc.h>

/* Reset the cpu by telling the reset controller to do so */
void reset_cpu(void)
{
	at91_rstc_t *rstc = (at91_rstc_t *) ATMEL_BASE_RSTC;

	writel(AT91_RSTC_KEY
		| AT91_RSTC_CR_PROCRST	/* Processor Reset */
		| AT91_RSTC_CR_PERRST	/* Peripheral Reset */
#ifdef CONFIG_AT91RESET_EXTRST
		| AT91_RSTC_CR_EXTRST	/* External Reset (assert nRST pin) */
#endif
		, &rstc->cr);
	/* never reached */
	while (1)
		;
}

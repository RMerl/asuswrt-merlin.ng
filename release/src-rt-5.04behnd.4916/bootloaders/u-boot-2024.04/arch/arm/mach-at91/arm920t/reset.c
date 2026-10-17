// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_st.h>

void  __attribute__((weak)) board_reset(void)
{
	/* true empty function for defining weak symbol */
}

void reset_cpu(void)
{
	at91_st_t *st = (at91_st_t *) ATMEL_BASE_ST;

	board_reset();

	/* Reset the cpu by setting up the watchdog timer */
	writel(AT91_ST_WDMR_RSTEN | AT91_ST_WDMR_EXTEN | AT91_ST_WDMR_WDV(2),
		&st->wdmr);
	writel(AT91_ST_CR_WDRST, &st->cr);
	/* and let it timeout */
	while (1)
		;
	/* Never reached */
}

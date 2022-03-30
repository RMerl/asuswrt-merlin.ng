// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2012
 * Markus Hubig <mhubig@imko.de>
 * IMKO GmbH <www.imko.de>
 *
 * Copyright (C) 2013 DENX Software Engineering, hs@denx.de
 */

#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <asm/arch/at91_rstc.h>
#include <watchdog.h>

void at91_phy_reset(void)
{
	unsigned long erstl;
	unsigned long start = get_timer(0);
	unsigned long const timeout = 1000; /* 1000ms */
	at91_rstc_t *rstc = (at91_rstc_t *)ATMEL_BASE_RSTC;

	erstl = readl(&rstc->mr) & AT91_RSTC_MR_ERSTL_MASK;

	/*
	 * Need to reset PHY -> 500ms reset
	 * Reset PHY by pulling the NRST line for 500ms to low. To do so
	 * disable user reset for low level on NRST pin and poll the NRST
	 * level in reset status register.
	 */
	writel(AT91_RSTC_KEY | AT91_RSTC_MR_ERSTL(0x0D) |
		AT91_RSTC_MR_URSTEN, &rstc->mr);

	writel(AT91_RSTC_KEY | AT91_RSTC_CR_EXTRST, &rstc->cr);

	/* Wait for end of hardware reset */
	while (!(readl(&rstc->sr) & AT91_RSTC_SR_NRSTL)) {
		/* avoid shutdown by watchdog */
		WATCHDOG_RESET();
		mdelay(10);

		/* timeout for not getting stuck in an endless loop */
		if (get_timer(start) >= timeout) {
			puts("*** ERROR: Timeout waiting for PHY reset!\n");
			break;
		}
	};

	/* Restore NRST value */
	writel(AT91_RSTC_KEY | erstl | AT91_RSTC_MR_URSTEN, &rstc->mr);
}

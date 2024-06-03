/*
 * watchdog.c - driver for i.mx on-chip watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#include <fsl_wdog.h>

#ifdef CONFIG_IMX_WATCHDOG
void hw_watchdog_reset(void)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
#endif /* CONFIG_WATCHDOG_RESET_DISABLE*/
}

void hw_watchdog_init(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;
	u16 timeout;

	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds. If not defined
	 * in configuration file, sets 128 Seconds
	 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif
	timeout = (CONFIG_WATCHDOG_TIMEOUT_MSECS / 500) - 1;
#ifdef CONFIG_FSL_LSCH2
	writew((WCR_WDA | WCR_SRS | WCR_WDE) << 8 | timeout, &wdog->wcr);
#else
	writew(WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_WDT | WCR_SRS |
		WCR_WDA | SET_WCR_WT(timeout), &wdog->wcr);
#endif /* CONFIG_FSL_LSCH2*/
	hw_watchdog_reset();
}
#endif

void __attribute__((weak)) reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	clrsetbits_le16(&wdog->wcr, WCR_WT_MSK, WCR_WDE);

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);	/* load minimum 1/2 second timeout */
	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}

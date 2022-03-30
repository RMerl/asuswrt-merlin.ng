// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2015 by Vladimir Zapolskiy <vz@mleia.com>
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/wdt.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

static struct clk_pm_regs *clk = (struct clk_pm_regs *)CLK_PM_BASE;
static struct wdt_regs  *wdt = (struct wdt_regs *)WDT_BASE;

void reset_cpu(ulong addr)
{
	/* Enable watchdog clock */
	setbits_le32(&clk->timclk_ctrl, CLK_TIMCLK_WATCHDOG);

	/* To be compatible with the original U-Boot code:
	 * addr: - 0: perform hard reset.
	 *       - !=0: perform a soft reset; i.e. "RESOUT_N" not asserted). */
	if (addr == 0) {
		/* Reset pulse length is 13005 peripheral clock frames */
		writel(13000, &wdt->pulse);

		/* Force WDOG_RESET2 and RESOUT_N signal active */
		writel(WDTIM_MCTRL_RESFRC2 | WDTIM_MCTRL_RESFRC1
		       | WDTIM_MCTRL_M_RES2, &wdt->mctrl);
	} else {
		/* Force match output active */
		writel(0x01, &wdt->emr);

		/* Internal reset on match output (no pulse on "RESOUT_N") */
		writel(WDTIM_MCTRL_M_RES1, &wdt->mctrl);
	}

	while (1)
		/* NOP */;
}

#if defined(CONFIG_ARCH_CPU_INIT)
int arch_cpu_init(void)
{
	/*
	 * It might be necessary to flush data cache, if U-Boot is loaded
	 * from kickstart bootloader, e.g. from S1L loader
	 */
	flush_dcache_all();

	return 0;
}
#else
#error "You have to select CONFIG_ARCH_CPU_INIT"
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	printf("CPU:   NXP LPC32XX\n");
	printf("CPU clock:        %uMHz\n", get_hclk_pll_rate() / 1000000);
	printf("AHB bus clock:    %uMHz\n", get_hclk_clk_rate() / 1000000);
	printf("Peripheral clock: %uMHz\n", get_periph_clk_rate() / 1000000);

	return 0;
}
#endif

#ifdef CONFIG_LPC32XX_ETH
int cpu_eth_init(bd_t *bis)
{
	lpc32xx_eth_initialize(bis);
	return 0;
}
#endif

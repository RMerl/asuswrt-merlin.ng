// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 * (C) Copyright 2009
 * Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_gpbr.h>
#include <asm/arch/clk.h>

#ifndef CONFIG_SYS_AT91_MAIN_CLOCK
#define CONFIG_SYS_AT91_MAIN_CLOCK 0
#endif

int arch_cpu_init(void)
{
	return at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);
}

void arch_preboot_os(void)
{
	ulong cpiv;
	at91_pit_t *pit = (at91_pit_t *) ATMEL_BASE_PIT;

	cpiv = AT91_PIT_MR_PIV_MASK(readl(&pit->piir));

	/*
	 * Disable PITC
	 * Add 0x1000 to current counter to stop it faster
	 * without waiting for wrapping back to 0
	 */
	writel(cpiv + 0x1000, &pit->mr);
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char __maybe_unused buf[32];

	printf("CPU: %s\n", ATMEL_CPU_NAME);
	printf("Crystal frequency: %8s MHz\n",
					strmhz(buf, get_main_clk_rate()));
	printf("CPU clock        : %8s MHz\n",
					strmhz(buf, get_cpu_clk_rate()));
	printf("Master clock     : %8s MHz\n",
					strmhz(buf, get_mck_clk_rate()));

	return 0;
}
#endif

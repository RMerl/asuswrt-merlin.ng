// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <asm/system.h>
#include <command.h>

/* Flush I/D-cache */
static void cache_flush(void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0" : : "r" (i));
}

int cleanup_before_linux(void)
{
	/*
	 * This function is called just before we call Linux. It prepares
	 * the processor for Linux by just disabling everything that can
	 * disturb booting Linux.
	 */

	disable_interrupts();
	icache_disable();
	dcache_disable();
	cache_flush();

	return 0;
}

void pxa_wait_ticks(int ticks)
{
	writel(0, OSCR);
	while (readl(OSCR) < ticks)
		asm volatile("" : : : "memory");
}

inline void writelrb(uint32_t val, uint32_t addr)
{
	writel(val, addr);
	asm volatile("" : : : "memory");
	readl(addr);
	asm volatile("" : : : "memory");
}

void pxa2xx_dram_init(void)
{
	uint32_t tmp;
	int i;
	/*
	 * 1) Initialize Asynchronous static memory controller
	 */

	writelrb(CONFIG_SYS_MSC0_VAL, MSC0);
	writelrb(CONFIG_SYS_MSC1_VAL, MSC1);
	writelrb(CONFIG_SYS_MSC2_VAL, MSC2);
	/*
	 * 2) Initialize Card Interface
	 */

	/* MECR: Memory Expansion Card Register */
	writelrb(CONFIG_SYS_MECR_VAL, MECR);
	/* MCMEM0: Card Interface slot 0 timing */
	writelrb(CONFIG_SYS_MCMEM0_VAL, MCMEM0);
	/* MCMEM1: Card Interface slot 1 timing */
	writelrb(CONFIG_SYS_MCMEM1_VAL, MCMEM1);
	/* MCATT0: Card Interface Attribute Space Timing, slot 0 */
	writelrb(CONFIG_SYS_MCATT0_VAL, MCATT0);
	/* MCATT1: Card Interface Attribute Space Timing, slot 1 */
	writelrb(CONFIG_SYS_MCATT1_VAL, MCATT1);
	/* MCIO0: Card Interface I/O Space Timing, slot 0 */
	writelrb(CONFIG_SYS_MCIO0_VAL, MCIO0);
	/* MCIO1: Card Interface I/O Space Timing, slot 1 */
	writelrb(CONFIG_SYS_MCIO1_VAL, MCIO1);

	/*
	 * 3) Configure Fly-By DMA register
	 */

	writelrb(CONFIG_SYS_FLYCNFG_VAL, FLYCNFG);

	/*
	 * 4) Initialize Timing for Sync Memory (SDCLK0)
	 */

	/*
	 * Before accessing MDREFR we need a valid DRI field, so we set
	 * this to power on defaults + DRI field.
	 */

	/* Read current MDREFR config and zero out DRI */
	tmp = readl(MDREFR) & ~0xfff;
	/* Add user-specified DRI */
	tmp |= CONFIG_SYS_MDREFR_VAL & 0xfff;
	/* Configure important bits */
	tmp |= MDREFR_K0RUN | MDREFR_SLFRSH;
	tmp &= ~(MDREFR_APD | MDREFR_E1PIN);

	/* Write MDREFR back */
	writelrb(tmp, MDREFR);

	/*
	 * 5) Initialize Synchronous Static Memory (Flash/Peripherals)
	 */

	/* Initialize SXCNFG register. Assert the enable bits.
	 *
	 * Write SXMRS to cause an MRS command to all enabled banks of
	 * synchronous static memory. Note that SXLCR need not be written
	 * at this time.
	 */
	writelrb(CONFIG_SYS_SXCNFG_VAL, SXCNFG);

	/*
	 * 6) Initialize SDRAM
	 */

	writelrb(CONFIG_SYS_MDREFR_VAL & ~MDREFR_SLFRSH, MDREFR);
	writelrb(CONFIG_SYS_MDREFR_VAL | MDREFR_E1PIN, MDREFR);

	/*
	 * 7) Write MDCNFG with MDCNFG:DEx deasserted (set to 0), to configure
	 *    but not enable each SDRAM partition pair.
	 */

	writelrb(CONFIG_SYS_MDCNFG_VAL &
		~(MDCNFG_DE0 | MDCNFG_DE1 | MDCNFG_DE2 | MDCNFG_DE3), MDCNFG);
	/* Wait for the clock to the SDRAMs to stabilize, 100..200 usec. */
	pxa_wait_ticks(0x300);

	/*
	 * 8) Trigger a number (usually 8) refresh cycles by attempting
	 *    non-burst read or write accesses to disabled SDRAM, as commonly
	 *    specified in the power up sequence documented in SDRAM data
	 *    sheets. The address(es) used for this purpose must not be
	 *    cacheable.
	 */
	for (i = 9; i >= 0; i--) {
		writel(i, 0xa0000000);
		asm volatile("" : : : "memory");
	}
	/*
	 * 9) Write MDCNFG with enable bits asserted (MDCNFG:DEx set to 1).
	 */

	tmp = CONFIG_SYS_MDCNFG_VAL &
		(MDCNFG_DE0 | MDCNFG_DE1 | MDCNFG_DE2 | MDCNFG_DE3);
	tmp |= readl(MDCNFG);
	writelrb(tmp, MDCNFG);

	/*
	 * 10) Write MDMRS.
	 */

	writelrb(CONFIG_SYS_MDMRS_VAL, MDMRS);

	/*
	 * 11) Enable APD
	 */

	if (CONFIG_SYS_MDREFR_VAL & MDREFR_APD) {
		tmp = readl(MDREFR);
		tmp |= MDREFR_APD;
		writelrb(tmp, MDREFR);
	}
}

void pxa_gpio_setup(void)
{
	writel(CONFIG_SYS_GPSR0_VAL, GPSR0);
	writel(CONFIG_SYS_GPSR1_VAL, GPSR1);
	writel(CONFIG_SYS_GPSR2_VAL, GPSR2);
#if defined(CONFIG_CPU_PXA27X)
	writel(CONFIG_SYS_GPSR3_VAL, GPSR3);
#endif

	writel(CONFIG_SYS_GPCR0_VAL, GPCR0);
	writel(CONFIG_SYS_GPCR1_VAL, GPCR1);
	writel(CONFIG_SYS_GPCR2_VAL, GPCR2);
#if defined(CONFIG_CPU_PXA27X)
	writel(CONFIG_SYS_GPCR3_VAL, GPCR3);
#endif

	writel(CONFIG_SYS_GPDR0_VAL, GPDR0);
	writel(CONFIG_SYS_GPDR1_VAL, GPDR1);
	writel(CONFIG_SYS_GPDR2_VAL, GPDR2);
#if defined(CONFIG_CPU_PXA27X)
	writel(CONFIG_SYS_GPDR3_VAL, GPDR3);
#endif

	writel(CONFIG_SYS_GAFR0_L_VAL, GAFR0_L);
	writel(CONFIG_SYS_GAFR0_U_VAL, GAFR0_U);
	writel(CONFIG_SYS_GAFR1_L_VAL, GAFR1_L);
	writel(CONFIG_SYS_GAFR1_U_VAL, GAFR1_U);
	writel(CONFIG_SYS_GAFR2_L_VAL, GAFR2_L);
	writel(CONFIG_SYS_GAFR2_U_VAL, GAFR2_U);
#if defined(CONFIG_CPU_PXA27X)
	writel(CONFIG_SYS_GAFR3_L_VAL, GAFR3_L);
	writel(CONFIG_SYS_GAFR3_U_VAL, GAFR3_U);
#endif

	writel(CONFIG_SYS_PSSR_VAL, PSSR);
}

void pxa_interrupt_setup(void)
{
	writel(0, ICLR);
	writel(0, ICMR);
#if defined(CONFIG_CPU_PXA27X)
	writel(0, ICLR2);
	writel(0, ICMR2);
#endif
}

void pxa_clock_setup(void)
{
	writel(CONFIG_SYS_CKEN, CKEN);
	writel(CONFIG_SYS_CCCR, CCCR);
	asm volatile("mcr	p14, 0, %0, c6, c0, 0" : : "r"(0x0b));

	/* enable the 32Khz oscillator for RTC and PowerManager */
	writel(OSCC_OON, OSCC);
	while (!(readl(OSCC) & OSCC_OOK))
		asm volatile("" : : : "memory");
}

void pxa_wakeup(void)
{
	uint32_t rcsr;

	rcsr = readl(RCSR);
	writel(rcsr & (RCSR_GPR | RCSR_SMR | RCSR_WDR | RCSR_HWR), RCSR);

	/* Wakeup */
	if (rcsr & RCSR_SMR) {
		writel(PSSR_PH, PSSR);
		pxa2xx_dram_init();
		icache_disable();
		dcache_disable();
		asm volatile("mov	pc, %0" : : "r"(readl(PSPR)));
	}
}

int arch_cpu_init(void)
{
	pxa_gpio_setup();
	pxa_wakeup();
	pxa_interrupt_setup();
	pxa_clock_setup();
	return 0;
}

void i2c_clk_enable(void)
{
	/* Set the global I2C clock on */
	writel(readl(CKEN) | CKEN14_I2C, CKEN);
}

void __attribute__((weak)) reset_cpu(ulong ignored) __attribute__((noreturn));

void reset_cpu(ulong ignored)
{
	uint32_t tmp;

	setbits_le32(OWER, OWER_WME);

	tmp = readl(OSCR);
	tmp += 0x1000;
	writel(tmp, OSMR3);
	writel(MDREFR_SLFRSH, MDREFR);

	for (;;)
		;
}

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}

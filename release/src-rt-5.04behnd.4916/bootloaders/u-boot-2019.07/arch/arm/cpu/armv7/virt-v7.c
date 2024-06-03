// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Andre Przywara, Linaro <andre.przywara@linaro.org>
 *
 * Routines to transition ARMv7 processors from secure into non-secure state
 * and from non-secure SVC into HYP mode
 * needed to enable ARMv7 virtualization for current hypervisors
 */

#include <common.h>
#include <asm/armv7.h>
#include <asm/gic.h>
#include <asm/io.h>
#include <asm/secure.h>

static unsigned int read_id_pfr1(void)
{
	unsigned int reg;

	asm("mrc p15, 0, %0, c0, c1, 1\n" : "=r"(reg));
	return reg;
}

static unsigned long get_gicd_base_address(void)
{
#ifdef CONFIG_ARM_GIC_BASE_ADDRESS
	return CONFIG_ARM_GIC_BASE_ADDRESS + GIC_DIST_OFFSET;
#else
	unsigned periphbase;

	/* get the GIC base address from the CBAR register */
	asm("mrc p15, 4, %0, c15, c0, 0\n" : "=r" (periphbase));

	/* the PERIPHBASE can be mapped above 4 GB (lower 8 bits used to
	 * encode this). Bail out here since we cannot access this without
	 * enabling paging.
	 */
	if ((periphbase & 0xff) != 0) {
		printf("nonsec: PERIPHBASE is above 4 GB, no access.\n");
		return -1;
	}

	return (periphbase & CBAR_MASK) + GIC_DIST_OFFSET;
#endif
}

/* Define a specific version of this function to enable any available
 * hardware protections for the reserved region */
void __weak protect_secure_section(void) {}

static void relocate_secure_section(void)
{
#ifdef CONFIG_ARMV7_SECURE_BASE
	size_t sz = __secure_end - __secure_start;
	unsigned long szflush = ALIGN(sz + 1, CONFIG_SYS_CACHELINE_SIZE);

	memcpy((void *)CONFIG_ARMV7_SECURE_BASE, __secure_start, sz);

	flush_dcache_range(CONFIG_ARMV7_SECURE_BASE,
			   CONFIG_ARMV7_SECURE_BASE + szflush);
	protect_secure_section();
	invalidate_icache_all();
#endif
}

static void kick_secondary_cpus_gic(unsigned long gicdaddr)
{
	/* kick all CPUs (except this one) by writing to GICD_SGIR */
	writel(1U << 24, gicdaddr + GICD_SGIR);
}

void __weak smp_kick_all_cpus(void)
{
	unsigned long gic_dist_addr;

	gic_dist_addr = get_gicd_base_address();
	if (gic_dist_addr == -1)
		return;

	kick_secondary_cpus_gic(gic_dist_addr);
}

__weak void psci_board_init(void)
{
}

int armv7_init_nonsec(void)
{
	unsigned int reg;
	unsigned itlinesnr, i;
	unsigned long gic_dist_addr;

	/* check whether the CPU supports the security extensions */
	reg = read_id_pfr1();
	if ((reg & 0xF0) == 0) {
		printf("nonsec: Security extensions not implemented.\n");
		return -1;
	}

	/* the SCR register will be set directly in the monitor mode handler,
	 * according to the spec one should not tinker with it in secure state
	 * in SVC mode. Do not try to read it once in non-secure state,
	 * any access to it will trap.
	 */

	gic_dist_addr = get_gicd_base_address();
	if (gic_dist_addr == -1)
		return -1;

	/* enable the GIC distributor */
	writel(readl(gic_dist_addr + GICD_CTLR) | 0x03,
	       gic_dist_addr + GICD_CTLR);

	/* TYPER[4:0] contains an encoded number of available interrupts */
	itlinesnr = readl(gic_dist_addr + GICD_TYPER) & 0x1f;

	/* set all bits in the GIC group registers to one to allow access
	 * from non-secure state. The first 32 interrupts are private per
	 * CPU and will be set later when enabling the GIC for each core
	 */
	for (i = 1; i <= itlinesnr; i++)
		writel((unsigned)-1, gic_dist_addr + GICD_IGROUPRn + 4 * i);

	psci_board_init();

	/*
	 * Relocate secure section before any cpu runs in secure ram.
	 * smp_kick_all_cpus may enable other cores and runs into secure
	 * ram, so need to relocate secure section before enabling other
	 * cores.
	 */
	relocate_secure_section();

#ifndef CONFIG_ARMV7_PSCI
	smp_set_core_boot_addr((unsigned long)secure_ram_addr(_smp_pen), -1);
	smp_kick_all_cpus();
#endif

	/* call the non-sec switching code on this CPU also */
	secure_ram_addr(_nonsec_init)();
	return 0;
}

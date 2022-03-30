// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#include <common.h>
#include <watchdog.h>
#include <asm/immap.h>
#include <asm/io.h>

#if defined(CONFIG_M5307)
/*
 * Simple mcf5307 chip select module init.
 *
 * Note: this chip has an issue reported in the device "errata":
 * MCF5307ER Rev 4.2 reports @ section 35:
 * Corrupted Return PC in Exception Stack Frame
 * When processing an autovectored interrupt an error can occur that
 * causes 0xFFFFFFFF to be written as the return PC value in the
 * exception stack frame. The problem is caused by a conflict between
 * an internal autovector access and a chip select mapped to the IACK
 * address space (0xFFFFXXXX).
 * Workaround:
 * Set the C/I bit in the chip select mask register (CSMR) for the
 * chip select that is mapped to 0xFFFFXXXX.
 * This will prevent the chip select from asserting for IACK accesses.
 */

#define MCF5307_SP_ERR_FIX(cs_base, mask)				\
	do {								\
		if (((cs_base<<16)+(in_be32(&mask)&0xffff0000)) >=	\
			0xffff0000)					\
			setbits_be32(&mask, CSMR_CI);			\
	} while (0)

void init_csm(void)
{
	csm_t *csm = (csm_t *)(MMAP_CSM);

#if (defined(CONFIG_SYS_CS0_BASE) && defined(CONFIG_SYS_CS0_MASK) && \
	defined(CONFIG_SYS_CS0_CTRL))
	out_be16(&csm->csar0, CONFIG_SYS_CS0_BASE);
	out_be32(&csm->csmr0, CONFIG_SYS_CS0_MASK);
	out_be16(&csm->cscr0, CONFIG_SYS_CS0_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS0_BASE, csm->csmr0);
#else
#warning "Chip Select 0 are not initialized/used"
#endif
#if (defined(CONFIG_SYS_CS1_BASE) && defined(CONFIG_SYS_CS1_MASK) && \
	defined(CONFIG_SYS_CS1_CTRL))
	out_be16(&csm->csar1, CONFIG_SYS_CS1_BASE);
	out_be32(&csm->csmr1, CONFIG_SYS_CS1_MASK);
	out_be16(&csm->cscr1, CONFIG_SYS_CS1_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS1_BASE, csm->csmr1);
#endif
#if (defined(CONFIG_SYS_CS2_BASE) && defined(CONFIG_SYS_CS2_MASK) && \
	defined(CONFIG_SYS_CS2_CTRL))
	out_be16(&csm->csar2, CONFIG_SYS_CS2_BASE);
	out_be32(&csm->csmr2, CONFIG_SYS_CS2_MASK);
	out_be16(&csm->cscr2, CONFIG_SYS_CS2_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS2_BASE, csm->csmr2);
#endif
#if (defined(CONFIG_SYS_CS3_BASE) && defined(CONFIG_SYS_CS3_MASK) && \
	defined(CONFIG_SYS_CS3_CTRL))
	out_be16(&csm->csar3, CONFIG_SYS_CS3_BASE);
	out_be32(&csm->csmr3, CONFIG_SYS_CS3_MASK);
	out_be16(&csm->cscr3, CONFIG_SYS_CS3_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS3_BASE, csm->csmr3);
#endif
#if (defined(CONFIG_SYS_CS4_BASE) && defined(CONFIG_SYS_CS4_MASK) && \
	defined(CONFIG_SYS_CS4_CTRL))
	out_be16(&csm->csar4, CONFIG_SYS_CS4_BASE);
	out_be32(&csm->csmr4, CONFIG_SYS_CS4_MASK);
	out_be16(&csm->cscr4, CONFIG_SYS_CS4_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS4_BASE, csm->csmr4);
#endif
#if (defined(CONFIG_SYS_CS5_BASE) && defined(CONFIG_SYS_CS5_MASK) && \
	defined(CONFIG_SYS_CS5_CTRL))
	out_be16(&csm->csar5, CONFIG_SYS_CS5_BASE);
	out_be32(&csm->csmr5, CONFIG_SYS_CS5_MASK);
	out_be16(&csm->cscr5, CONFIG_SYS_CS5_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS5_BASE, csm->csmr5);
#endif
#if (defined(CONFIG_SYS_CS6_BASE) && defined(CONFIG_SYS_CS6_MASK) && \
	defined(CONFIG_SYS_CS6_CTRL))
	out_be16(&csm->csar6, CONFIG_SYS_CS6_BASE);
	out_be32(&csm->csmr6, CONFIG_SYS_CS6_MASK);
	out_be16(&csm->cscr6, CONFIG_SYS_CS6_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS6_BASE, csm->csmr6);
#endif
#if (defined(CONFIG_SYS_CS7_BASE) && defined(CONFIG_SYS_CS7_MASK) && \
	defined(CONFIG_SYS_CS7_CTRL))
	out_be16(&csm->csar7, CONFIG_SYS_CS7_BASE);
	out_be32(&csm->csmr7, CONFIG_SYS_CS7_MASK);
	out_be16(&csm->cscr7, CONFIG_SYS_CS7_CTRL);
	MCF5307_SP_ERR_FIX(CONFIG_SYS_CS7_BASE, csm->csmr7);
#endif
}

/*
 * Set up the memory map and initialize registers
 */
void cpu_init_f(void)
{
	sim_t *sim = (sim_t *)(MMAP_SIM);

	out_8(&sim->sypcr, 0x00);
	out_8(&sim->swivr, 0x0f);
	out_8(&sim->swsr,  0x00);
	out_8(&sim->mpark, 0x00);

	intctrl_t *icr = (intctrl_t *)(MMAP_INTC);

	/* timer 2 not masked */
	out_be32(&icr->imr, 0xfffffbff);

	out_8(&icr->icr0, 0x00); /* sw watchdog */
	out_8(&icr->icr1, 0x00); /* timer 1     */
	out_8(&icr->icr2, 0x88); /* timer 2     */
	out_8(&icr->icr3, 0x00); /* i2c         */
	out_8(&icr->icr4, 0x00); /* uart 0      */
	out_8(&icr->icr5, 0x00); /* uart 1      */
	out_8(&icr->icr6, 0x00); /* dma  0      */
	out_8(&icr->icr7, 0x00); /* dma  1      */
	out_8(&icr->icr8, 0x00); /* dma  2      */
	out_8(&icr->icr9, 0x00); /* dma  3      */

	/* Chipselect Init */
	init_csm();

	/* enable data/instruction cache now */
	icache_enable();
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	return 0;
}

void uart_port_conf(int port)
{
}

void arch_preboot_os(void)
{
	/*
	 * OS can change interrupt offsets and are about to boot the OS so
	 * we need to make sure we disable all async interrupts.
	 */
	intctrl_t *icr = (intctrl_t *)(MMAP_INTC);

	out_8(&icr->icr1, 0x00); /* timer 1     */
	out_8(&icr->icr2, 0x00); /* timer 2     */
}
#endif

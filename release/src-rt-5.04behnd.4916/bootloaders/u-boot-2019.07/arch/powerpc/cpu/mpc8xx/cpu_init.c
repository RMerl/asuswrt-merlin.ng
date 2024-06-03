// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <watchdog.h>

#include <mpc8xx.h>
#include <asm/cpm_8xx.h>
#include <asm/io.h>

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void cpu_init_f(immap_t __iomem *immr)
{
	memctl8xx_t __iomem *memctl = &immr->im_memctl;
	ulong reg;

	/* SYPCR - contains watchdog control (11-9) */

#ifndef CONFIG_HW_WATCHDOG
	/* deactivate watchdog if not enabled in config */
	out_be32(&immr->im_siu_conf.sc_sypcr, CONFIG_SYS_SYPCR & ~SYPCR_SWE);
#endif

	WATCHDOG_RESET();

	/* SIUMCR - contains debug pin configuration (11-6) */
	setbits_be32(&immr->im_siu_conf.sc_siumcr, CONFIG_SYS_SIUMCR);
	/* initialize timebase status and control register (11-26) */
	/* unlock TBSCRK */

	out_be32(&immr->im_sitk.sitk_tbscrk, KAPWR_KEY);
	out_be16(&immr->im_sit.sit_tbscr, CONFIG_SYS_TBSCR | TBSCR_TBE);

	/* Unlock timebase register */
	out_be32(&immr->im_sitk.sitk_tbk, KAPWR_KEY);

	/* initialize the PIT (11-31) */

	out_be32(&immr->im_sitk.sitk_piscrk, KAPWR_KEY);
	out_be16(&immr->im_sit.sit_piscr, CONFIG_SYS_PISCR);

	/* System integration timers. Don't change EBDF! (15-27) */

	out_be32(&immr->im_clkrstk.cark_sccrk, KAPWR_KEY);
	clrsetbits_be32(&immr->im_clkrst.car_sccr, ~CONFIG_SYS_SCCR_MASK,
			CONFIG_SYS_SCCR);

	/*
	 * MPC866/885 ERRATA GLL2
	 * Description:
	 *   In 1:2:1 mode, when HRESET is detected at the positive edge of
	 *   EXTCLK, then there will be a loss of phase between
	 *   EXTCLK and CLKOUT.
	 *
	 * Workaround:
	 *   Reprogram the SCCR:
	 *   1.   Write 1'b00 to SCCR[EBDF].
	 *   2.   Write 1'b01 to SCCR[EBDF].
	 *   3.   Rewrite the desired value to the PLPRCR register.
	 */
	reg = in_be32(&immr->im_clkrst.car_sccr);
	/* Are we in mode 1:2:1 ? */
	if ((reg & SCCR_EBDF11) == SCCR_EBDF01) {
		clrbits_be32(&immr->im_clkrst.car_sccr, SCCR_EBDF11);
		setbits_be32(&immr->im_clkrst.car_sccr, SCCR_EBDF01);
	}

	/* PLL (CPU clock) settings (15-30) */

	out_be32(&immr->im_clkrstk.cark_plprcrk, KAPWR_KEY);

	/* If CONFIG_SYS_PLPRCR (set in the various *_config.h files) tries to
	 * set the MF field, then just copy CONFIG_SYS_PLPRCR over car_plprcr,
	 * otherwise OR in CONFIG_SYS_PLPRCR so we do not change the current MF
	 * field value.
	 *
	 * For newer (starting MPC866) chips PLPRCR layout is different.
	 */
#ifdef CONFIG_SYS_PLPRCR
	if ((CONFIG_SYS_PLPRCR & PLPRCR_MFACT_MSK) != 0) /* reset control bits*/
		out_be32(&immr->im_clkrst.car_plprcr, CONFIG_SYS_PLPRCR);
	else /* isolate MF-related fields and reset control bits */
		clrsetbits_be32(&immr->im_clkrst.car_plprcr, ~PLPRCR_MFACT_MSK,
				CONFIG_SYS_PLPRCR);
#endif

	/*
	 * Memory Controller:
	 */

	/* Clear everything except Port Size bits & add the "Bank Valid" bit */
	clrsetbits_be32(&memctl->memc_br0, ~BR_PS_MSK, BR_V);

	/* Map banks 0 (and maybe 1) to the FLASH banks 0 (and 1) at
	 * preliminary addresses - these have to be modified later
	 * when FLASH size has been determined
	 *
	 * Depending on the size of the memory region defined by
	 * CONFIG_SYS_OR0_REMAP some boards (wide address mask) allow to map the
	 * CONFIG_SYS_MONITOR_BASE, while others (narrower address mask) can't
	 * map CONFIG_SYS_MONITOR_BASE.
	 *
	 * For example, for CONFIG_IVMS8, the CONFIG_SYS_MONITOR_BASE is
	 * 0xff000000, but CONFIG_SYS_OR0_REMAP's address mask is 0xfff80000.
	 *
	 * If BR0 wasn't loaded with address base 0xff000000, then BR0's
	 * base address remains as 0x00000000. However, the address mask
	 * have been narrowed to 512Kb, so CONFIG_SYS_MONITOR_BASE wasn't mapped
	 * into the Bank0.
	 *
	 * This is why CONFIG_IVMS8 and similar boards must load BR0 with
	 * CONFIG_SYS_BR0_PRELIM in advance.
	 *
	 * [Thanks to Michael Liao for this explanation.
	 *  I owe him a free beer. - wd]
	 */

#if defined(CONFIG_SYS_OR0_REMAP)
	out_be32(&memctl->memc_or0, CONFIG_SYS_OR0_REMAP);
#endif
#if defined(CONFIG_SYS_OR1_REMAP)
	out_be32(&memctl->memc_or1, CONFIG_SYS_OR1_REMAP);
#endif
#if defined(CONFIG_SYS_OR5_REMAP)
	out_be32(&memctl->memc_or5, CONFIG_SYS_OR5_REMAP);
#endif

	/* now restrict to preliminary range */
	out_be32(&memctl->memc_br0, CONFIG_SYS_BR0_PRELIM);
	out_be32(&memctl->memc_or0, CONFIG_SYS_OR0_PRELIM);

#if (defined(CONFIG_SYS_OR1_PRELIM) && defined(CONFIG_SYS_BR1_PRELIM))
	out_be32(&memctl->memc_or1, CONFIG_SYS_OR1_PRELIM);
	out_be32(&memctl->memc_br1, CONFIG_SYS_BR1_PRELIM);
#endif

#if defined(CONFIG_SYS_OR2_PRELIM) && defined(CONFIG_SYS_BR2_PRELIM)
	out_be32(&memctl->memc_or2, CONFIG_SYS_OR2_PRELIM);
	out_be32(&memctl->memc_br2, CONFIG_SYS_BR2_PRELIM);
#endif

#if defined(CONFIG_SYS_OR3_PRELIM) && defined(CONFIG_SYS_BR3_PRELIM)
	out_be32(&memctl->memc_or3, CONFIG_SYS_OR3_PRELIM);
	out_be32(&memctl->memc_br3, CONFIG_SYS_BR3_PRELIM);
#endif

#if defined(CONFIG_SYS_OR4_PRELIM) && defined(CONFIG_SYS_BR4_PRELIM)
	out_be32(&memctl->memc_or4, CONFIG_SYS_OR4_PRELIM);
	out_be32(&memctl->memc_br4, CONFIG_SYS_BR4_PRELIM);
#endif

#if defined(CONFIG_SYS_OR5_PRELIM) && defined(CONFIG_SYS_BR5_PRELIM)
	out_be32(&memctl->memc_or5, CONFIG_SYS_OR5_PRELIM);
	out_be32(&memctl->memc_br5, CONFIG_SYS_BR5_PRELIM);
#endif

#if defined(CONFIG_SYS_OR6_PRELIM) && defined(CONFIG_SYS_BR6_PRELIM)
	out_be32(&memctl->memc_or6, CONFIG_SYS_OR6_PRELIM);
	out_be32(&memctl->memc_br6, CONFIG_SYS_BR6_PRELIM);
#endif

#if defined(CONFIG_SYS_OR7_PRELIM) && defined(CONFIG_SYS_BR7_PRELIM)
	out_be32(&memctl->memc_or7, CONFIG_SYS_OR7_PRELIM);
	out_be32(&memctl->memc_br7, CONFIG_SYS_BR7_PRELIM);
#endif

	/*
	 * Reset CPM
	 */
	out_be16(&immr->im_cpm.cp_cpcr, CPM_CR_RST | CPM_CR_FLG);
	/* Spin until command processed */
	while (in_be16(&immr->im_cpm.cp_cpcr) & CPM_CR_FLG)
		;
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r(void)
{
	return 0;
}

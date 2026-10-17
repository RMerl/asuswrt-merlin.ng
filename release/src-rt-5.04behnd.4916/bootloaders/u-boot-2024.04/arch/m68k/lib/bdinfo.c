// SPDX-License-Identifier: GPL-2.0+
/*
 * PPC-specific information for the 'bd' command
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <config.h>
#include <init.h>
#include <asm/u-boot.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_setup_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	bd->bi_mbar_base = CFG_SYS_MBAR; /* base of internal registers */

	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */

	if (IS_ENABLED(CONFIG_PCI))
		bd->bi_pcifreq = gd->pci_clk;

#if defined(CONFIG_EXTRA_CLOCK)
	bd->bi_inpfreq = gd->arch.inp_clk;	/* input Freq in Hz */
	bd->bi_vcofreq = gd->arch.vco_clk;	/* vco Freq in Hz */
	bd->bi_flbfreq = gd->arch.flb_clk;	/* flexbus Freq in Hz */
#endif

	return 0;
}

void arch_print_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	bdinfo_print_mhz("busfreq", bd->bi_busfreq);
#if defined(CFG_SYS_MBAR)
	bdinfo_print_num_l("mbar", bd->bi_mbar_base);
#endif
	bdinfo_print_mhz("cpufreq", bd->bi_intfreq);
	if (IS_ENABLED(CONFIG_PCI))
		bdinfo_print_mhz("pcifreq", bd->bi_pcifreq);
#ifdef CONFIG_EXTRA_CLOCK
	bdinfo_print_mhz("flbfreq", bd->bi_flbfreq);
	bdinfo_print_mhz("inpfreq", bd->bi_inpfreq);
	bdinfo_print_mhz("vcofreq", bd->bi_vcofreq);
#endif
}

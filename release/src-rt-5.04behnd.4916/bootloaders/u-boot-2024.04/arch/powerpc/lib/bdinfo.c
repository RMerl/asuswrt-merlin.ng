// SPDX-License-Identifier: GPL-2.0+
/*
 * PPC-specific information for the 'bd' command
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_setup_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

#if defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
	bd->bi_immr_base = CONFIG_SYS_IMMR;	/* base  of IMMR register     */
#endif

#if defined(CONFIG_MPC83xx)
	bd->bi_immrbar = CONFIG_SYS_IMMR;
#endif

	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */

	return 0;
}

void __weak board_detail(void)
{
	/* Please define board_detail() for your PPC platform */
}

void arch_print_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	bdinfo_print_mhz("busfreq", bd->bi_busfreq);
#if defined(CONFIG_MPC8xx) || defined(CONFIG_E500)
	bdinfo_print_num_l("immr_base", bd->bi_immr_base);
#endif
	bdinfo_print_num_l("bootflags", bd->bi_bootflags);
	bdinfo_print_mhz("intfreq", bd->bi_intfreq);
#ifdef CONFIG_ENABLE_36BIT_PHYS
	if (IS_ENABLED(CONFIG_PHYS_64BIT))
		puts("addressing  = 36-bit\n");
	else
		puts("addressing  = 32-bit\n");
#endif
	board_detail();
}

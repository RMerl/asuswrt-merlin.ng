// SPDX-License-Identifier: GPL-2.0+
/*
 * ARM-specific information for the 'bd' command
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_setup_bdinfo(void)
{
#ifdef CONFIG_MACH_TYPE
	struct bd_info *bd = gd->bd;

	bd->bi_arch_number = CONFIG_MACH_TYPE; /* board id for Linux */
#endif

	return 0;
}

void arch_print_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	bdinfo_print_num_l("arch_number", bd->bi_arch_number);
#ifdef CFG_SYS_MEM_RESERVE_SECURE
	if (gd->arch.secure_ram & MEM_RESERVE_SECURE_SECURED) {
		bdinfo_print_num_ll("Secure ram",
				    gd->arch.secure_ram &
				    MEM_RESERVE_SECURE_ADDR_MASK);
	}
#endif
#ifdef CONFIG_RESV_RAM
	if (gd->arch.resv_ram)
		bdinfo_print_num_ll("Reserved ram", gd->arch.resv_ram);
#endif
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
	bdinfo_print_num_l("TLB addr", gd->arch.tlb_addr);
#endif
	bdinfo_print_num_l("irq_sp", gd->irq_sp);	/* irq stack pointer */
	bdinfo_print_num_l("sp start ", gd->start_addr_sp);
	/*
	 * TODO: Currently only support for davinci SOC's is added.
	 * Remove this check once all the board implement this.
	 */
#ifdef CONFIG_CLOCKS
	printf("ARM frequency = %ld MHz\n", bd->bi_arm_freq);
	printf("DSP frequency = %ld MHz\n", bd->bi_dsp_freq);
	printf("DDR frequency = %ld MHz\n", bd->bi_ddr_freq);
#endif
#ifdef CONFIG_BOARD_TYPES
	printf("Board Type  = %ld\n", gd->board_type);
#endif
#if CONFIG_IS_ENABLED(SYS_MALLOC_F)
	printf("Early malloc usage: %lx / %x\n", gd->malloc_ptr,
	       CONFIG_VAL(SYS_MALLOC_F_LEN));
#endif
}

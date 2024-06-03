// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <hwconfig.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_fdt.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include "../common/sleep.h"
#include "t104xrdb.h"
#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	u8 sw;

#if defined(CONFIG_TARGET_T1040D4RDB) || defined(CONFIG_TARGET_T1042D4RDB)
	printf("Board: %sD4RDB\n", cpu->name);
#else
	printf("Board: %sRDB\n", cpu->name);
#endif
	printf("Board rev: 0x%02x CPLD ver: 0x%02x, ",
	       CPLD_READ(hw_ver), CPLD_READ(sw_ver));

	sw = CPLD_READ(flash_ctl_status);
	sw = ((sw & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);

	printf("vBank: %d\n", sw);

	return 0;
}

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_SYS_FLASH_BASE
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);
#endif
	return 0;
}

int misc_init_r(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) >> 24;

	printf("SERDES Reference : 0x%X\n", srds_s1);

	/* select SGMII*/
	if (srds_s1 == 0x86)
		CPLD_WRITE(misc_ctl_status, CPLD_READ(misc_ctl_status) |
					 MISC_CTL_SG_SEL);

	/* select SGMII and Aurora*/
	if (srds_s1 == 0x8E)
		CPLD_WRITE(misc_ctl_status, CPLD_READ(misc_ctl_status) |
					 MISC_CTL_SG_SEL | MISC_CTL_AURORA_SEL);

#if defined(CONFIG_TARGET_T1040D4RDB)
	if (hwconfig("qe-tdm")) {
		CPLD_WRITE(sfp_ctl_status, CPLD_READ(sfp_ctl_status) |
			   MISC_MUX_QE_TDM);
		printf("QECSR : 0x%02x, mux to qe-tdm\n",
		       CPLD_READ(sfp_ctl_status));
	}
	/* Mask all CPLD interrupt sources, except QSGMII interrupts */
	if (CPLD_READ(sw_ver) < 0x03) {
		debug("CPLD SW version 0x%02x doesn't support int_mask\n",
		      CPLD_READ(sw_ver));
	} else {
		CPLD_WRITE(int_mask, CPLD_INT_MASK_ALL &
			   ~(CPLD_INT_MASK_QSGMII1 | CPLD_INT_MASK_QSGMII2));
	}
#endif

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	if (hwconfig("qe-tdm"))
		fdt_del_diu(blob);
	return 0;
}

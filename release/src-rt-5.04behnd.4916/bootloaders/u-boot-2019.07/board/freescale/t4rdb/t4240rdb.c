// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "t4rdb.h"
#include "cpld.h"
#include "../common/vid.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	u8 sw;

	printf("Board: %sRDB, ", cpu->name);
	printf("Board rev: 0x%02x CPLD ver: 0x%02x%02x, ",
	       CPLD_READ(hw_ver), CPLD_READ(sw_maj_ver), CPLD_READ(sw_min_ver));

	sw = CPLD_READ(vbank);
	sw = sw & CPLD_BANK_SEL_MASK;

	if (sw <= 7)
		printf("vBank: %d\n", sw);
	else
		printf("Unsupported Bank=%x\n", sw);

	puts("SERDES Reference Clocks:\n");
	printf("       SERDES1=100MHz SERDES2=156.25MHz\n"
	       "       SERDES3=100MHz SERDES4=100MHz\n");

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
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
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	/*
	 * Adjust core voltage according to voltage ID
	 * This function changes I2C mux to channel 2.
	*/
	if (adjust_vdd(0))
		printf("Warning: Adjusting core voltage failed.\n");

	return 0;
}

int misc_init_r(void)
{
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
	fsl_fdt_fixup_dr_usb(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}

/*
 * This function is called by bdinfo to print detail board information.
 * As an exmaple for future board, we organize the messages into
 * several sections. If applicable, the message is in the format of
 * <name>      = <value>
 * It should aligned with normal output of bdinfo command.
 *
 * Voltage: Core, DDR and another configurable voltages
 * Clock  : Critical clocks which are not printed already
 * RCW    : RCW source if not printed already
 * Misc   : Other important information not in above catagories
 */
void board_detail(void)
{
	int rcwsrc;

	/* RCW section SW3[4] */
	rcwsrc = 0x0;
	puts("RCW source  = ");
	switch (rcwsrc & 0x1) {
	case 0x1:
		puts("SDHC/eMMC\n");
		break;
	default:
		puts("I2C normal addressing\n");
		break;
	}
}

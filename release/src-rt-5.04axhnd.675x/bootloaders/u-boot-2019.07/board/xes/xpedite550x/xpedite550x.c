// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Extreme Engineering Solutions, Inc.
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <pca953x.h>

extern void ft_board_pci_setup(void *blob, bd_t *bd);

static void flash_cs_fixup(void)
{
	int flash_sel;

	/*
	 * Print boot dev and swap flash flash chip selects if booted from 2nd
	 * flash.  Swapping chip selects presents user with a common memory
	 * map regardless of which flash was booted from.
	 */
	flash_sel = !((pca953x_get_val(CONFIG_SYS_I2C_PCA953X_ADDR0) &
			CONFIG_SYS_PCA953X_C0_FLASH_PASS_CS));
	printf("Flash: Executed from flash%d\n", flash_sel ? 2 : 1);

	if (flash_sel) {
		set_lbc_br(0, CONFIG_SYS_BR1_PRELIM);
		set_lbc_or(0, CONFIG_SYS_OR1_PRELIM);

		set_lbc_br(1, CONFIG_SYS_BR0_PRELIM);
		set_lbc_or(1, CONFIG_SYS_OR0_PRELIM);
	}
}

int board_early_init_r(void)
{
	/* Initialize PCA9557 devices */
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR0, 0xff, 0);
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR1, 0xff, 0);
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR2, 0xff, 0);
	pca953x_set_pol(CONFIG_SYS_I2C_PCA953X_ADDR3, 0xff, 0);

	/*
	 * Remap NOR flash region to caching-inhibited
	 * so that flash can be erased/programmed properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* Invalidate existing TLB entry for NOR flash */
	disable_tlb(0);
	set_tlb(1, (CONFIG_SYS_FLASH_BASE2 & 0xf0000000),
		(CONFIG_SYS_FLASH_BASE2 & 0xf0000000),
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, 0, BOOKE_PAGESZ_256M, 1);

	flash_cs_fixup();

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_PCI
	ft_board_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif

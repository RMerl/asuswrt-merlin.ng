// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2013 Freescale Semiconductor, Inc.
 * Copyright 2021-2023 NXP
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <fdt_support.h>
#include <i2c.h>
#include <image.h>
#include <init.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <clock_legacy.h>
#include <fm_eth.h>
#include "t208xrdb.h"
#include "cpld.h"
#include "../common/vid.h"

DECLARE_GLOBAL_DATA_PTR;

u8 get_hw_revision(void)
{
	u8 ver = CPLD_READ(hw_ver);

	switch (ver) {
	default:
	case 0x1:
		return 'C';
	case 0x0:
		return 'D';
	case 0x2:
		return 'E';
	}
}

#if CONFIG_IS_ENABLED(DM_SERIAL)
int get_serial_clock(void)
{
	return get_bus_freq(0) / 2;
}
#endif

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *freq[3] = {"100.00MHZ", "125.00MHz", "156.25MHZ"};

	printf("Board: %sRDB, ", cpu->name);
	printf("Board rev: %c CPLD ver: 0x%02x, boot from ",
	       get_hw_revision(), CPLD_READ(sw_ver));

#ifdef CONFIG_SDCARD
	puts("SD/MMC\n");
#elif CONFIG_SPIFLASH
	puts("SPI\n");
#else
	u8 reg;

	reg = CPLD_READ(flash_csr);

	if (reg & CPLD_BOOT_SEL) {
		puts("NAND\n");
	} else {
		reg = ((reg & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);
		printf("NOR vBank%d\n", reg);
	}
#endif

	puts("SERDES Reference Clocks:\n");
	printf("SD1_CLK1=%s, SD1_CLK2=%s\n", freq[2], freq[0]);
	printf("SD2_CLK1=%s, SD2_CLK2=%s\n", freq[0], freq[0]);

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CFG_SYS_FLASH_BASE;
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

	set_tlb(1, flashbase, CFG_SYS_FLASH_BASE_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	/*
	 * Adjust core voltage according to voltage ID
	 * This function changes I2C mux to channel 2.
	 */
	if (adjust_vdd(0))
		printf("Warning: Adjusting core voltage failed.\n");

	pci_init();

	return 0;
}

int misc_init_r(void)
{
	u8 reg;

	/* Reset CS4315 PHY */
	reg = CPLD_READ(reset_ctl);
	reg |= CPLD_RSTCON_EDC_RST;
	CPLD_WRITE(reset_ctl, reg);

	/* Enable POR for boards revisions D and up */
	if (get_hw_revision() >= 'D') {
		reg = CPLD_READ(misc_csr);
		reg |= CPLD_MISC_POR_EN;
		CPLD_WRITE(misc_csr, reg);
	}

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
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
	fdt_fixup_board_fman_ethernet(blob);
	fdt_fixup_board_enet(blob);
	fdt_fixup_board_phy(blob);
#endif

	return 0;
}

ulong *cs4340_get_fw_addr(void)
{
	ulong cortina_fw_addr = CONFIG_CORTINA_FW_ADDR;

#ifdef CONFIG_SYS_CORTINA_FW_IN_NOR
	u8 reg;

	reg = CPLD_READ(flash_csr);
	if (!(reg & CPLD_BOOT_SEL)) {
		reg = ((reg & CPLD_LBMAP_MASK) >> CPLD_LBMAP_SHIFT);
		if (reg == 0)
			cortina_fw_addr = CORTINA_FW_ADDR_IFCNOR;
		else if (reg == 4)
			cortina_fw_addr = CORTINA_FW_ADDR_IFCNOR_ALTBANK;
	}
#endif

	return (ulong *)cortina_fw_addr;
}

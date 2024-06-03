// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
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

#include "../common/ngpixis.h"
#include "corenet_ds.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void)
{
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
#if defined(CONFIG_TARGET_P3041DS) || defined(CONFIG_TARGET_P5020DS) || \
	defined(CONFIG_TARGET_P5040DS)
	unsigned int i;
#endif
	static const char * const freq[] = {"100", "125", "156.25", "212.5" };

	printf("Board: %sDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(&pixis->id), in_8(&pixis->arch), in_8(&pixis->scver));

	sw = in_8(&PIXIS_SW(PIXIS_LBMAP_SWITCH));
	sw = (sw & PIXIS_LBMAP_MASK) >> PIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("Promjet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else
		printf("invalid setting of SW%u\n", PIXIS_LBMAP_SWITCH);

	/* Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES Reference Clocks: ");
#if defined(CONFIG_TARGET_P3041DS) || defined(CONFIG_TARGET_P5020DS) || \
	defined(CONFIG_TARGET_P5040DS)
	sw = in_8(&PIXIS_SW(5));
	for (i = 0; i < 3; i++) {
		unsigned int clock = (sw >> (6 - (2 * i))) & 3;

		printf("Bank%u=%sMhz ", i+1, freq[clock]);
	}
#ifdef CONFIG_TARGET_P5040DS
	/* On P5040DS, SW11[7:8] determines the Bank 4 frequency */
	sw = in_8(&PIXIS_SW(9));
	printf("Bank4=%sMhz ", freq[sw & 3]);
#endif
	puts("\n");
#else
	sw = in_8(&PIXIS_SW(3));
	/* SW3[2]: 0 = 100 Mhz, 1 = 125 MHz */
	/* SW3[3]: 0 = 125 Mhz, 1 = 156.25 MHz */
	/* SW3[4]: 0 = 125 Mhz, 1 = 156.25 MHz */
	printf("Bank1=%sMHz ", freq[!!(sw & 0x40)]);
	printf("Bank2=%sMHz ", freq[1 + !!(sw & 0x20)]);
	printf("Bank3=%sMHz\n", freq[1 + !!(sw & 0x10)]);
#endif

	return 0;
}

int board_early_init_f(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/*
	 * P4080 DS board only uses the DDR1_MCK0/3 and DDR2_MCK0/3
	 * disable the DDR1_MCK1/2/4/5 and DDR2_MCK1/2/4/5 to reduce
	 * the noise introduced by these unterminated and unused clock pairs.
	 */
	setbits_be32(&gur->ddrclkdr, 0x001B001B);

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

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,	/* tlb, epn, rpn */
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,	/* perms, wimge */
			0, flash_esel, BOOKE_PAGESZ_256M, 1);	/* ts, esel, tsize, iprot */

	return 0;
}

#define NUM_SRDS_BANKS	3

int misc_init_r(void)
{
	serdes_corenet_t *srds_regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	u8 sw;

#if defined(CONFIG_TARGET_P3041DS) || defined(CONFIG_TARGET_P5020DS) || \
	defined(CONFIG_TARGET_P5040DS)
	sw = in_8(&PIXIS_SW(5));
	for (i = 0; i < 3; i++) {
		unsigned int clock = (sw >> (6 - (2 * i))) & 3;
		switch (clock) {
		case 0:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_100;
			break;
		case 1:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		case 2:
			actual[i] = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		default:
			printf("Warning: SDREFCLK%u switch setting of '11' is "
			       "unsupported\n", i + 1);
			break;
		}
	}
#else
	/* Warn if the expected SERDES reference clocks don't match the
	 * actual reference clocks.  This needs to be done after calling
	 * p4080_erratum_serdes8(), since that function may modify the clocks.
	 */
	sw = in_8(&PIXIS_SW(3));
	actual[0] = (sw & 0x40) ?
		SRDS_PLLCR0_RFCK_SEL_125 : SRDS_PLLCR0_RFCK_SEL_100;
	actual[1] = (sw & 0x20) ?
		SRDS_PLLCR0_RFCK_SEL_156_25 : SRDS_PLLCR0_RFCK_SEL_125;
	actual[2] = (sw & 0x10) ?
		SRDS_PLLCR0_RFCK_SEL_156_25 : SRDS_PLLCR0_RFCK_SEL_125;
#endif

	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 expected = srds_regs->bank[i].pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

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

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
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
#include <hwconfig.h>

#include "../common/sleep.h"
#include "../common/qixis.h"
#include "t1040qds.h"
#include "t1040qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	char buf[64];
	u8 sw;
	struct cpu_type *cpu = gd->arch.cpu;
	static const char *const freq[] = {"100", "125", "156.25", "161.13",
						"122.88", "122.88", "122.88"};
	int clock;

	printf("Board: %sQDS, ", cpu->name);
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x, ",
	       QIXIS_READ(id), QIXIS_READ(arch));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("PromJet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else if (sw == 0x15)
		printf("IFCCard\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);

	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES Reference: ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = (sw >> 6) & 3;
	printf("Clock1=%sMHz ", freq[clock]);
	clock = (sw >> 4) & 3;
	printf("Clock2=%sMHz\n", freq[clock]);

	return 0;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

static void qe_board_setup(void)
{
	u8 brdcfg15, brdcfg9;

	if (hwconfig("qe") && hwconfig("tdm")) {
		brdcfg15 = QIXIS_READ(brdcfg[15]);
		/*
		 * TDMRiser uses QE-TDM
		 * Route QE_TDM signals to TDM Riser slot
		 */
		QIXIS_WRITE(brdcfg[15], brdcfg15 | 7);
	} else if (hwconfig("qe") && hwconfig("uart")) {
		brdcfg15 = QIXIS_READ(brdcfg[15]);
		brdcfg9 = QIXIS_READ(brdcfg[9]);
		/*
		 * Route QE_TDM signals to UCC
		 * ProfiBus controlled by UCC3
		 */
		brdcfg15 &= 0xfc;
		QIXIS_WRITE(brdcfg[15], brdcfg15 | 2);
		QIXIS_WRITE(brdcfg[9], brdcfg9 | 4);
	}
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
#endif
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);

	return 0;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0F) {
	case QIXIS_SYSCLK_64:
		return 64000000;
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 66666666;
}

#define NUM_SRDS_BANKS	2
int misc_init_r(void)
{
	u8 sw;
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS] = { 0 };
	int i;

	sw = QIXIS_READ(brdcfg[2]);
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		unsigned int clock = (sw >> (6 - 2 * i)) & 3;
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
		}
	}

	puts("SerDes1");
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 pllcr0 = srds_regs->bank[i].pllcr0;
		u32 expected = pllcr0 & SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("expects ref clk%d %sMHz, but actual is %sMHz\n",
			       i + 1, serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

	qe_board_setup();

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
	fdt_fixup_board_enet(blob);
#endif

	return 0;
}

void qixis_dump_switch(void)
{
	int i, nr_of_cfgsw;

	QIXIS_WRITE(cms[0], 0x00);
	nr_of_cfgsw = QIXIS_READ(cms[1]);

	puts("DIP switch settings dump:\n");
	for (i = 1; i <= nr_of_cfgsw; i++) {
		QIXIS_WRITE(cms[0], i);
		printf("SW%d = (0x%02x)\n", i, QIXIS_READ(cms[1]));
	}
}

int board_need_mem_reset(void)
{
	return 1;
}

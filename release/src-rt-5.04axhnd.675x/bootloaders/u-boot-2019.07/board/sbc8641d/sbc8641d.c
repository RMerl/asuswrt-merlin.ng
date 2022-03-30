// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007 Wind River Systemes, Inc. <www.windriver.com>
 * Copyright 2007 Embedded Specialties, Inc.
 * Joe Hamman joe.hamman@embeddedspecialties.com
 *
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_86xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

long int fixed_sdram (void);

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	puts ("Board: Wind River SBC8641D\n");

	return 0;
}

int dram_init(void)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = fsl_ddr_sdram();
#else
	dram_size = fixed_sdram ();
#endif

	debug ("    DDR: ");
	gd->ram_size = dram_size;

	return 0;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	puts ("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts ("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	puts ("SDRAM test passed.\n");
	return 0;
}
#endif

#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
long int fixed_sdram (void)
{
#if !defined(CONFIG_SYS_RAMBOOT)
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile struct ccsr_ddr *ddr = &immap->im_ddr1;

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs1_bnds = CONFIG_SYS_DDR_CS1_BNDS;
	ddr->cs2_bnds = CONFIG_SYS_DDR_CS2_BNDS;
	ddr->cs3_bnds = CONFIG_SYS_DDR_CS3_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;
	ddr->cs1_config = CONFIG_SYS_DDR_CS1_CONFIG;
	ddr->cs2_config = CONFIG_SYS_DDR_CS2_CONFIG;
	ddr->cs3_config = CONFIG_SYS_DDR_CS3_CONFIG;
	ddr->timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_cfg = CONFIG_SYS_DDR_CFG_1A;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR_CFG_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR_MODE_2;
	ddr->sdram_md_cntl = CONFIG_SYS_DDR_MODE_CTL;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg = CONFIG_SYS_DDR_CFG_1B;
	asm ("sync; isync");

	udelay (500);
	ddr = &immap->im_ddr2;

	ddr->cs0_bnds = CONFIG_SYS_DDR2_CS0_BNDS;
	ddr->cs1_bnds = CONFIG_SYS_DDR2_CS1_BNDS;
	ddr->cs2_bnds = CONFIG_SYS_DDR2_CS2_BNDS;
	ddr->cs3_bnds = CONFIG_SYS_DDR2_CS3_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR2_CS0_CONFIG;
	ddr->cs1_config = CONFIG_SYS_DDR2_CS1_CONFIG;
	ddr->cs2_config = CONFIG_SYS_DDR2_CS2_CONFIG;
	ddr->cs3_config = CONFIG_SYS_DDR2_CS3_CONFIG;
	ddr->timing_cfg_3 = CONFIG_SYS_DDR2_EXT_REFRESH;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR2_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR2_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR2_TIMING_2;
	ddr->sdram_cfg = CONFIG_SYS_DDR2_CFG_1A;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR2_CFG_2;
	ddr->sdram_mode = CONFIG_SYS_DDR2_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR2_MODE_2;
	ddr->sdram_md_cntl = CONFIG_SYS_DDR2_MODE_CTL;
	ddr->sdram_interval = CONFIG_SYS_DDR2_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR2_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR2_CLK_CTRL;

	asm ("sync;isync");

	udelay (500);

	ddr->sdram_cfg = CONFIG_SYS_DDR2_CFG_1B;
	asm ("sync; isync");

	udelay (500);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif				/* !defined(CONFIG_SPD_EEPROM) */

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif /* CONFIG_PCI */


#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

	return 0;
}
#endif

void sbc8641d_reset_board (void)
{
	puts ("Resetting board....\n");
}

/*
 * get_board_sys_clk
 *      Clock is fixed at 1GHz on this board. Used for CONFIG_SYS_CLK_FREQ
 */

unsigned long get_board_sys_clk (ulong dummy)
{
	int i;
	ulong val = 0;

	i = 5;
	i &= 0x07;

	switch (i) {
	case 0:
		val = 33000000;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66000000;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 134000000;
		break;
	case 7:
		val = 166000000;
		break;
	}

	return val;
}

void board_reset(void)
{
#ifdef CONFIG_SYS_RESET_ADDRESS
	ulong addr = CONFIG_SYS_RESET_ADDRESS;

	/* flush and disable I/D cache */
	__asm__ __volatile__ ("mfspr	3, 1008"	::: "r3");
	__asm__ __volatile__ ("ori	5, 5, 0xcc00"	::: "r5");
	__asm__ __volatile__ ("ori	4, 3, 0xc00"	::: "r4");
	__asm__ __volatile__ ("andc	5, 3, 5"	::: "r5");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 4");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 5");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");

	/*
	 * SRR0 has system reset vector, SRR1 has default MSR value
	 * rfi restores MSR from SRR1 and sets the PC to the SRR0 value
	 */
	__asm__ __volatile__ ("mtspr	26, %0"		:: "r" (addr));
	__asm__ __volatile__ ("li	4, (1 << 6)"	::: "r4");
	__asm__ __volatile__ ("mtspr	27, 4");
	__asm__ __volatile__ ("rfi");
#endif
}

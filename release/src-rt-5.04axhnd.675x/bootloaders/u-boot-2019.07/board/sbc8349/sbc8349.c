// SPDX-License-Identifier: GPL-2.0+
/*
 * sbc8349.c -- WindRiver SBC8349 board support.
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * Paul Gortmaker <paul.gortmaker@windriver.com>
 * Based on board/mpc8349emds/mpc8349emds.c (and previous 834x releases.)
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#include <i2c.h>
#include <spd_sdram.h>
#include <miiphy.h>
#if defined(CONFIG_OF_LIBFDT)
#include <linux/libfdt.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int fixed_sdram(void);
void sdram_init(void);

#if defined(CONFIG_DDR_ECC) && defined(CONFIG_MPC83xx)
void ddr_enable_ecc(unsigned int dram_size);
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f (void)
{
	return 0;
}
#endif

#define ns2clk(ns) (ns / (1000000000 / CONFIG_8349_CLKIN) + 1)

int dram_init(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_SDRAM_BASE & LAWBAR_BAR;
#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif
	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	sdram_init();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/* set total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize * 1024 * 1024;

	return 0;
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE;
	u32 ddr_size = msize << 20;	/* DDR size in bytes */
	u32 ddr_size_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar = LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);

#if (CONFIG_SYS_DDR_SIZE != 256)
#warning Currently any ddr size other than 256 is not supported
#endif

#if ((CONFIG_SYS_SDRAM_BASE & 0x00FFFFFF) != 0)
#warning Chip select bounds is only configurable in 16MB increments
#endif
	im->ddr.csbnds[2].csbnds =
		((CONFIG_SYS_SDRAM_BASE >> CSBNDS_SA_SHIFT) & CSBNDS_SA) |
		(((CONFIG_SYS_SDRAM_BASE + ddr_size - 1) >>
				CSBNDS_EA_SHIFT) & CSBNDS_EA);
	im->ddr.cs_config[2] = CONFIG_SYS_DDR_CS2_CONFIG;

	/* currently we use only one CS, so disable the other banks */
	im->ddr.cs_config[0] = 0;
	im->ddr.cs_config[1] = 0;
	im->ddr.cs_config[3] = 0;

	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;

	im->ddr.sdram_cfg =
		SDRAM_CFG_SREN
#if defined(CONFIG_DDR_2T_TIMING)
		| SDRAM_CFG_2T_EN
#endif
		| SDRAM_CFG_SDRAM_TYPE_DDR1;
#if defined (CONFIG_DDR_32BIT)
	/* for 32-bit mode burst length is 8 */
	im->ddr.sdram_cfg |= (SDRAM_CFG_32_BE | SDRAM_CFG_8_BE);
#endif
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;

	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	udelay(200);

	/* enable DDR controller */
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	return msize;
}
#endif/*!CONFIG_SYS_SPD_EEPROM*/


int checkboard (void)
{
	puts("Board: Wind River SBC834x\n");
	return 0;
}

/*
 * if board is fitted with SDRAM
 */
#if defined(CONFIG_SYS_BR2_PRELIM)  \
	&& defined(CONFIG_SYS_OR2_PRELIM) \
	&& defined(CONFIG_SYS_LBLAWBAR2_PRELIM) \
	&& defined(CONFIG_SYS_LBLAWAR2_PRELIM)
/*
 * Initialize SDRAM memory on the Local Bus.
 */

void sdram_init(void)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile fsl_lbc_t *lbc = &immap->im_lbc;
	uint *sdram_addr = (uint *)CONFIG_SYS_LBC_SDRAM_BASE;
	const u32 lsdmr_common = LSDMR_RFEN | LSDMR_BSMA1516 | LSDMR_RFCR8 |
				 LSDMR_PRETOACT6 | LSDMR_ACTTORW3 | LSDMR_BL8 |
				 LSDMR_WRC3 | LSDMR_CL3;

	puts("\n   SDRAM on Local Bus: ");
	print_size (CONFIG_SYS_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */

	/* setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = 0x00000000;
	/* LB refresh timer prescal, 266MHz/32 */
	lbc->mrtpr = 0x20000000;
	/* LB sdram refresh timer, about 6us */
	lbc->lsrt = 0x32000000;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	/* 0x40636733; normal operation */
	lbc->lsdmr = lsdmr_common | LSDMR_OP_NORMAL;

	/* 0x68636733; precharge all the banks */
	lbc->lsdmr = lsdmr_common | LSDMR_OP_PCHALL;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/* 0x48636733; auto refresh */
	lbc->lsdmr = lsdmr_common | LSDMR_OP_ARFRSH;
	asm("sync");
	/*1 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*2 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*3 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*4 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*5 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*6 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*7 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*8 times*/
	*sdram_addr = 0xff;
	udelay(100);

	/* 0x58636733; mode register write operation */
	lbc->lsdmr = lsdmr_common | LSDMR_OP_MRW;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/* 0x40636733; normal operation */
	lbc->lsdmr = lsdmr_common | LSDMR_OP_NORMAL;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);
}
#else
void sdram_init(void)
{
	puts("   SDRAM on Local Bus: Disabled in config\n");
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	return 0;
}
#endif

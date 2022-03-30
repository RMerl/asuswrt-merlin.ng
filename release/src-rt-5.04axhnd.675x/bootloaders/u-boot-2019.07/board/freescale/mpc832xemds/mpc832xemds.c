// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <miiphy.h>
#include <command.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#include <asm/mmu.h>
#if defined(CONFIG_OF_LIBFDT)
#include <linux/libfdt.h>
#endif
#if defined(CONFIG_PQ_MDS_PIB)
#include "../common/pq-mds-pib.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* ETH3 */
	{1,  0, 1, 0, 1}, /* TxD0 */
	{1,  1, 1, 0, 1}, /* TxD1 */
	{1,  2, 1, 0, 1}, /* TxD2 */
	{1,  3, 1, 0, 1}, /* TxD3 */
	{1,  9, 1, 0, 1}, /* TxER */
	{1, 12, 1, 0, 1}, /* TxEN */
	{3, 24, 2, 0, 1}, /* TxCLK->CLK10 */

	{1,  4, 2, 0, 1}, /* RxD0 */
	{1,  5, 2, 0, 1}, /* RxD1 */
	{1,  6, 2, 0, 1}, /* RxD2 */
	{1,  7, 2, 0, 1}, /* RxD3 */
	{1,  8, 2, 0, 1}, /* RxER */
	{1, 10, 2, 0, 1}, /* RxDV */
	{0, 13, 2, 0, 1}, /* RxCLK->CLK9 */
	{1, 11, 2, 0, 1}, /* COL */
	{1, 13, 2, 0, 1}, /* CRS */

	/* ETH4 */
	{1, 18, 1, 0, 1}, /* TxD0 */
	{1, 19, 1, 0, 1}, /* TxD1 */
	{1, 20, 1, 0, 1}, /* TxD2 */
	{1, 21, 1, 0, 1}, /* TxD3 */
	{1, 27, 1, 0, 1}, /* TxER */
	{1, 30, 1, 0, 1}, /* TxEN */
	{3,  6, 2, 0, 1}, /* TxCLK->CLK8 */

	{1, 22, 2, 0, 1}, /* RxD0 */
	{1, 23, 2, 0, 1}, /* RxD1 */
	{1, 24, 2, 0, 1}, /* RxD2 */
	{1, 25, 2, 0, 1}, /* RxD3 */
	{1, 26, 1, 0, 1}, /* RxER */
	{1, 28, 2, 0, 1}, /* Rx_DV */
	{3, 31, 2, 0, 1}, /* RxCLK->CLK7 */
	{1, 29, 2, 0, 1}, /* COL */
	{1, 31, 2, 0, 1}, /* CRS */

	{3,  4, 3, 0, 2}, /* MDIO */
	{3,  5, 1, 0, 2}, /* MDC */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

int board_early_init_f(void)
{
	volatile u8 *bcsr = (volatile u8 *)CONFIG_SYS_BCSR;

	/* Enable flash write */
	bcsr[9] &= ~0x08;

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_PQ_MDS_PIB
	pib_init();
#endif
	return 0;
}

int fixed_sdram(void);

int dram_init(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -ENXIO;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_SDRAM_BASE & LAWBAR_BAR;

	msize = fixed_sdram();

	/* set total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize * 1024 * 1024;

	return 0;
}

/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CONFIG_SYS_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1) {
			return -1;
		}
	}
	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
#if (CONFIG_SYS_DDR_SIZE != 128)
#warning Currenly any ddr size other than 128 is not supported
#endif
	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CNTL;
	im->ddr.csbnds[0].csbnds = CONFIG_SYS_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	__asm__ __volatile__ ("sync");
	udelay(200);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	__asm__ __volatile__ ("sync");
	return msize;
}

int checkboard(void)
{
	puts("Board: Freescale MPC832XEMDS\n");
	return 0;
}

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

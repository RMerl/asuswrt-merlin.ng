/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Michael Barkowski <michael.barkowski@freescale.com>
 * Based on mpc832xmds file by Dave Liu <daveliu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <miiphy.h>
#include <command.h>
#include <linux/libfdt.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* UCC3 */
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

	/* UCC2 */
	{0, 18, 1, 0, 1}, /* TxD0 */
	{0, 19, 1, 0, 1}, /* TxD1 */
	{0, 20, 1, 0, 1}, /* TxD2 */
	{0, 21, 1, 0, 1}, /* TxD3 */
	{0, 27, 1, 0, 1}, /* TxER */
	{0, 30, 1, 0, 1}, /* TxEN */
	{3, 23, 2, 0, 1}, /* TxCLK->CLK3 */

	{0, 22, 2, 0, 1}, /* RxD0 */
	{0, 23, 2, 0, 1}, /* RxD1 */
	{0, 24, 2, 0, 1}, /* RxD2 */
	{0, 25, 2, 0, 1}, /* RxD3 */
	{0, 26, 1, 0, 1}, /* RxER */
	{0, 28, 2, 0, 1}, /* Rx_DV */
	{3, 21, 2, 0, 1}, /* RxCLK->CLK16 */
	{0, 29, 2, 0, 1}, /* COL */
	{0, 31, 2, 0, 1}, /* CRS */

	{3,  4, 3, 0, 2}, /* MDIO */
	{3,  5, 1, 0, 2}, /* MDC */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

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
	puts("Board: Freescale MPC8323ERDB\n");
	return 0;
}

static struct pci_region pci_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI1_MEM_BASE,
		phys_start: CONFIG_SYS_PCI1_MEM_PHYS,
		size: CONFIG_SYS_PCI1_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI1_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI1_MMIO_PHYS,
		size: CONFIG_SYS_PCI1_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CONFIG_SYS_PCI1_IO_BASE,
		phys_start: CONFIG_SYS_PCI1_IO_PHYS,
		size: CONFIG_SYS_PCI1_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci_regions };

	/* Enable all 3 PCI_CLK_OUTPUTs. */
	clk->occr |= 0xe0000000;

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg);
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

#if defined(CONFIG_SYS_I2C_MAC_OFFSET)
int mac_read_from_eeprom(void)
{
	uchar buf[28];
	char str[18];
	int i = 0;
	unsigned int crc = 0;
	unsigned char enetvar[32];

	/* Read MAC addresses from EEPROM */
	if (eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, CONFIG_SYS_I2C_MAC_OFFSET, buf, 28)) {
		printf("\nEEPROM @ 0x%02x read FAILED!!!\n",
		       CONFIG_SYS_I2C_EEPROM_ADDR);
	} else {
		uint32_t crc_buf;

		memcpy(&crc_buf, &buf[24], sizeof(uint32_t));

		if (crc32(crc, buf, 24) == crc_buf) {
			printf("Reading MAC from EEPROM\n");
			for (i = 0; i < 4; i++) {
				if (memcmp(&buf[i * 6], "\0\0\0\0\0\0", 6)) {
					sprintf(str,
						"%02X:%02X:%02X:%02X:%02X:%02X",
						buf[i * 6], buf[i * 6 + 1],
						buf[i * 6 + 2], buf[i * 6 + 3],
						buf[i * 6 + 4], buf[i * 6 + 5]);
					sprintf((char *)enetvar,
						i ? "eth%daddr" : "ethaddr", i);
					env_set((char *)enetvar, str);
				}
			}
		}
	}
	return 0;
}
#endif				/* CONFIG_I2C_MAC_OFFSET */

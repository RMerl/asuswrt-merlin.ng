// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2007
 *
 * Author: Scott Wood <scottwood@freescale.com>
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <pci.h>
#include <mpc83xx.h>
#include <ns16550.h>
#include <nand.h>

#include <asm/bitops.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

extern void disable_addr_trans (void);
extern void enable_addr_trans (void);

int checkboard(void)
{
	puts("Board: ve8313\n");
	return 0;
}

static long fixed_sdram(void)
{
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;

#ifndef CONFIG_SYS_RAMBOOT
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	u32 msize_log2 = __ilog2(msize);

	out_be32(&im->sysconf.ddrlaw[0].bar,
		(CONFIG_SYS_SDRAM_BASE & 0xfffff000));
	out_be32(&im->sysconf.ddrlaw[0].ar, (LBLAWAR_EN | (msize_log2 - 1)));
	out_be32(&im->sysconf.ddrcdr, CONFIG_SYS_DDRCDR_VALUE);

	/*
	 * Erratum DDR3 requires a 50ms delay after clearing DDRCDR[DDR_cfg],
	 * or the DDR2 controller may fail to initialize correctly.
	 */
	__udelay(50000);

#if ((CONFIG_SYS_SDRAM_BASE & 0x00FFFFFF) != 0)
#warning Chip select bounds is only configurable in 16MB increments
#endif
	out_be32(&im->ddr.csbnds[0].csbnds,
		((CONFIG_SYS_SDRAM_BASE >> CSBNDS_SA_SHIFT) & CSBNDS_SA) |
		(((CONFIG_SYS_SDRAM_BASE + msize - 1) >> CSBNDS_EA_SHIFT) &
			CSBNDS_EA));
	out_be32(&im->ddr.cs_config[0], CONFIG_SYS_DDR_CS0_CONFIG);

	/* Currently we use only one CS, so disable the other bank. */
	out_be32(&im->ddr.cs_config[1], 0);

	out_be32(&im->ddr.sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CNTL);
	out_be32(&im->ddr.timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&im->ddr.timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&im->ddr.timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&im->ddr.timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);

	out_be32(&im->ddr.sdram_cfg, CONFIG_SYS_SDRAM_CFG);

	out_be32(&im->ddr.sdram_cfg2, CONFIG_SYS_SDRAM_CFG2);
	out_be32(&im->ddr.sdram_mode, CONFIG_SYS_DDR_MODE);
	out_be32(&im->ddr.sdram_mode2, CONFIG_SYS_DDR_MODE_2);

	out_be32(&im->ddr.sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	sync();

	/* enable DDR controller */
	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);

	/* now check the real size */
	disable_addr_trans ();
	msize = get_ram_size (CONFIG_SYS_SDRAM_BASE, msize);
	enable_addr_trans ();
#endif

	return msize;
}

int dram_init(void)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile fsl_lbc_t *lbc = &im->im_lbc;
	u32 msize;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	msize = fixed_sdram();

	/* Local Bus setup lbcr and mrtpr */
	out_be32(&lbc->lbcr, 0x00040000);
	out_be32(&lbc->mrtpr, 0x20000000);
	sync();

	/* return total bus SDRAM size(bytes)  -- DDR */
	gd->ram_size = msize;

	return 0;
}

#define VE8313_WDT_EN	0x00020000
#define VE8313_WDT_TRIG	0x00040000

int board_early_init_f (void)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)im->gpio;

#if defined(CONFIG_HW_WATCHDOG)
	/* enable WDT */
	clrbits_be32(&gpio->dat, VE8313_WDT_EN | VE8313_WDT_TRIG);
#else
	/* disable WDT */
	setbits_be32(&gpio->dat, VE8313_WDT_EN | VE8313_WDT_TRIG);
#endif
	/* set WDT pins as output */
	setbits_be32(&gpio->dir, VE8313_WDT_EN | VE8313_WDT_TRIG);

	return 0;
}

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	volatile immap_t *im = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (volatile gpio83xx_t *)im->gpio;
	unsigned long reg;

	reg = in_be32(&gpio->dat);
	if (reg & VE8313_WDT_TRIG)
		clrbits_be32(&gpio->dat, VE8313_WDT_TRIG);
	else
		setbits_be32(&gpio->dat, VE8313_WDT_TRIG);
}
#endif


#if defined(CONFIG_PCI)
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
	setbits_be32(&clk->occr, 0xe0000000);

	/*
	 * Configure PCI Local Access Windows
	 */
	out_be32(&pci_law[0].bar, CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR);
	out_be32(&pci_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	out_be32(&pci_law[1].bar, CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR);
	out_be32(&pci_law[1].ar, LBLAWAR_EN | LBLAWAR_1MB);

	mpc83xx_pci_init(1, reg);
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

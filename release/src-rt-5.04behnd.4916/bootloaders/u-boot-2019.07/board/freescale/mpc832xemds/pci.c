// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 */

/*
 * PCI Configuration space access support for MPC83xx PCI Bridge
 */
#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <mpc83xx.h>
#include <pci.h>
#include <i2c.h>
#include <asm/fsl_i2c.h>
#include "../common/pq-mds-pib.h"

static struct pci_region pci1_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI1_MEM_BASE,
		phys_start: CONFIG_SYS_PCI1_MEM_PHYS,
		size: CONFIG_SYS_PCI1_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI1_IO_BASE,
		phys_start: CONFIG_SYS_PCI1_IO_PHYS,
		size: CONFIG_SYS_PCI1_IO_SIZE,
		flags: PCI_REGION_IO
	},
	{
		bus_start: CONFIG_SYS_PCI1_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI1_MMIO_PHYS,
		size: CONFIG_SYS_PCI1_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
};

#ifdef CONFIG_MPC83XX_PCI2
static struct pci_region pci2_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI2_MEM_BASE,
		phys_start: CONFIG_SYS_PCI2_MEM_PHYS,
		size: CONFIG_SYS_PCI2_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI2_IO_BASE,
		phys_start: CONFIG_SYS_PCI2_IO_PHYS,
		size: CONFIG_SYS_PCI2_IO_SIZE,
		flags: PCI_REGION_IO
	},
	{
		bus_start: CONFIG_SYS_PCI2_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI2_MMIO_PHYS,
		size: CONFIG_SYS_PCI2_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
};
#endif

void pci_init_board(void)
#ifdef CONFIG_PCISLAVE
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	volatile pcictrl83xx_t *pci_ctrl = &immr->pci_ctrl[0];
	struct pci_region *reg[] = { pci1_regions };

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_1G;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_4M;

	mpc83xx_pci_init(1, reg);

	/*
	 * Configure PCI Inbound Translation Windows
	 */
	pci_ctrl[0].pitar0 = 0x0;
	pci_ctrl[0].pibar0 = 0x0;
	pci_ctrl[0].piwar0 = PIWAR_EN | PIWAR_RTT_SNOOP |
	    PIWAR_WTT_SNOOP | PIWAR_IWS_4K;

	pci_ctrl[0].pitar1 = 0x0;
	pci_ctrl[0].pibar1 = 0x0;
	pci_ctrl[0].piebar1 = 0x0;
	pci_ctrl[0].piwar1 &= ~PIWAR_EN;

	pci_ctrl[0].pitar2 = 0x0;
	pci_ctrl[0].pibar2 = 0x0;
	pci_ctrl[0].piebar2 = 0x0;
	pci_ctrl[0].piwar2 &= ~PIWAR_EN;

	/* Unlock the configuration bit */
	mpc83xx_pcislave_unlock(0);
	printf("PCI:   Agent mode enabled\n");
}
#else
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
#ifndef CONFIG_MPC83XX_PCI2
	struct pci_region *reg[] = { pci1_regions };
#else
	struct pci_region *reg[] = { pci1_regions, pci2_regions };
#endif

	/* initialize the PCA9555PW IO expander on the PIB board */
	pib_init();

#if defined(CONFIG_PCI_66M)
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2;
	printf("PCI clock is 66MHz\n");
#elif defined(CONFIG_PCI_33M)
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2 |
	    OCCR_PCICD0 | OCCR_PCICD1 | OCCR_PCICD2 | OCCR_PCICR;
	printf("PCI clock is 33MHz\n");
#else
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2;
	printf("PCI clock is 66MHz\n");
#endif
	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_512M;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_1M;

	udelay(2000);

#ifndef CONFIG_MPC83XX_PCI2
	mpc83xx_pci_init(1, reg);
#else
	mpc83xx_pci_init(2, reg);
#endif
}
#endif				/* CONFIG_PCISLAVE */

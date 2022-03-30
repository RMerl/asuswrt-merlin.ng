// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 */

#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <mpc83xx.h>
#include <pci.h>
#include <i2c.h>
#include <asm/fsl_i2c.h>

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

/*
 * pci_init_board()
 *
 * NOTICE: MPC8349 internally has two PCI controllers (PCI1 and PCI2) but since
 * per TQM834x design physical connections to external devices (PCI sockets)
 * are routed only to the PCI1 we do not account for the second one - this code
 * supports PCI1 module only. Should support for the PCI2 be required in the
 * future it needs a separate pci_controller structure (above) and handling -
 * please refer to other boards' implementation for dual PCI host controllers,
 * for example board/Marvell/db64360/pci.c, pci_init_board()
 *
 */
void
pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci1_regions };
	u32 reg32;

	/*
	 * Configure PCI controller and PCI_CLK_OUTPUT
	 *
	 * WARNING! only PCI_CLK_OUTPUT1 is enabled here as this is the one
	 * line actually used for clocking all external PCI devices in TQM83xx.
	 * Enabling other PCI_CLK_OUTPUT lines may lead to board's hang for
	 * unknown reasons - particularly PCI_CLK_OUTPUT6 and PCI_CLK_OUTPUT7
	 * are known to hang the board; this issue is under investigation
	 * (13 oct 05)
	 */
	reg32 = OCCR_PCICOE1;
#if 0
	/* enabling all PCI_CLK_OUTPUT lines HANGS the board... */
	reg32 = 0xff000000;
#endif
	if (clk->spmr & SPMR_CKID) {
		/* PCI Clock is half CONFIG_SYS_CLK_FREQ so need to set up OCCR
		 * fields accordingly */
		reg32 |= (OCCR_PCI1CR | OCCR_PCI2CR);

		reg32 |= (OCCR_PCICD0 | OCCR_PCICD1 | OCCR_PCICD2 \
			  | OCCR_PCICD3 | OCCR_PCICD4 | OCCR_PCICD5 \
			  | OCCR_PCICD6 | OCCR_PCICD7);
	}

	clk->occr = reg32;
	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_512M;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_16M;

	udelay(2000);

	mpc83xx_pci_init(1, reg);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 */

#include <common.h>

#include <asm/mmu.h>
#include <asm/io.h>
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
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
#ifndef CONFIG_MPC83XX_PCI2
	struct pci_region *reg[] = { pci1_regions };
#else
	struct pci_region *reg[] = { pci1_regions, pci2_regions };
#endif
	u8 reg8;

#if defined(CONFIG_SYS_I2C)
	i2c_set_bus_num(1);
	/* Read the PCI_M66EN jumper setting */
	if ((i2c_read(CONFIG_SYS_I2C_8574_ADDR2, 0, 0, &reg8, sizeof(reg8)) == 0) ||
	    (i2c_read(CONFIG_SYS_I2C_8574A_ADDR2, 0, 0, &reg8, sizeof(reg8)) == 0)) {
		if (reg8 & I2C_8574_PCI66)
			clk->occr = 0xff000000;	/* 66 MHz PCI */
		else
			clk->occr = 0xff600001;	/* 33 MHz PCI */
	} else {
		clk->occr = 0xff600001;	/* 33 MHz PCI */
	}
#else
	clk->occr = 0xff000000;	/* 66 MHz PCI */
#endif
	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_1G;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_32M;

	udelay(2000);

#ifndef CONFIG_MPC83XX_PCI2
	mpc83xx_pci_init(1, reg);
#else
	mpc83xx_pci_init(2, reg);
#endif
}

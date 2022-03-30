// SPDX-License-Identifier: GPL-2.0+
/*
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

#ifndef CONFIG_PCISLAVE
void pib_init(void)
{
	u8 val8, orig_i2c_bus;
	/*
	 * Assign PIB PMC slot to desired PCI bus
	 */
	/* Switch temporarily to I2C bus #2 */
	orig_i2c_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	val8 = 0;
	i2c_write(0x23, 0x6, 1, &val8, 1);
	i2c_write(0x23, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x23, 0x2, 1, &val8, 1);
	i2c_write(0x23, 0x3, 1, &val8, 1);

	val8 = 0;
	i2c_write(0x26, 0x6, 1, &val8, 1);
	val8 = 0x34;
	i2c_write(0x26, 0x7, 1, &val8, 1);
#if defined(CONFIG_PCI_64BIT)
	val8 = 0xf4;	/* PMC2:PCI1/64-bit */
#elif defined(CONFIG_PCI_ALL_PCI1)
	val8 = 0xf3;	/* PMC1:PCI1 PMC2:PCI1 PMC3:PCI1 */
#elif defined(CONFIG_PCI_ONE_PCI1)
	val8 = 0xf9;	/* PMC1:PCI1 PMC2:PCI2 PMC3:PCI2 */
#else
	val8 = 0xf5;	/* PMC1:PCI1 PMC2:PCI1 PMC3:PCI2 */
#endif
	i2c_write(0x26, 0x2, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x26, 0x3, 1, &val8, 1);
	val8 = 0;
	i2c_write(0x27, 0x6, 1, &val8, 1);
	i2c_write(0x27, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x27, 0x2, 1, &val8, 1);
	val8 = 0xef;
	i2c_write(0x27, 0x3, 1, &val8, 1);
	asm("eieio");

#if defined(CONFIG_PCI_64BIT)
	printf("PCI1: 64-bit on PMC2\n");
#elif defined(CONFIG_PCI_ALL_PCI1)
	printf("PCI1: 32-bit on PMC1, PMC2, PMC3\n");
#elif defined(CONFIG_PCI_ONE_PCI1)
	printf("PCI1: 32-bit on PMC1\n");
	printf("PCI2: 32-bit on PMC2, PMC3\n");
#else
	printf("PCI1: 32-bit on PMC1, PMC2\n");
	printf("PCI2: 32-bit on PMC3\n");
#endif
	/* Reset to original I2C bus */
	i2c_set_bus_num(orig_i2c_bus);
}

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

	/* initialize the PCA9555PW IO expander on the PIB board */
	pib_init();

	/* Enable all 8 PCI_CLK_OUTPUTS */
	clk->occr = 0xff000000;
	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_1G;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_4M;

	udelay(2000);

#ifndef CONFIG_MPC83XX_PCI2
	mpc83xx_pci_init(1, reg);
#else
	mpc83xx_pci_init(2, reg);
#endif
}

#else
void pci_init_board(void)
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

	/* Configure PCI Inbound Translation Windows (3 1MB windows) */
	pci_ctrl->pitar0 = 0x0;
	pci_ctrl->pibar0 = 0x0;
	pci_ctrl->piwar0 = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP |
			   PIWAR_WTT_SNOOP | PIWAR_IWS_1M;

	pci_ctrl->pitar1  = 0x0;
	pci_ctrl->pibar1  = 0x0;
	pci_ctrl->piebar1 = 0x0;
	pci_ctrl->piwar1  = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP |
			    PIWAR_WTT_SNOOP | PIWAR_IWS_1M;

	pci_ctrl->pitar2  = 0x0;
	pci_ctrl->pibar2  = 0x0;
	pci_ctrl->piebar2 = 0x0;
	pci_ctrl->piwar2  = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP |
			    PIWAR_WTT_SNOOP | PIWAR_IWS_1M;

	/* Unlock the configuration bit */
	mpc83xx_pcislave_unlock(0);
	printf("PCI:   Agent mode enabled\n");
}
#endif /* CONFIG_PCISLAVE */

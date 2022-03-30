// SPDX-License-Identifier: GPL-2.0+
/*
 * pci.c -- esd VME8349 PCI board support.
 * Copyright (c) 2006 Wind River Systems, Inc.
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * Based on MPC8349 PCI support but w/o PIB related code.
 */

#include <asm/mmu.h>
#include <asm/io.h>
#include <common.h>
#include <mpc83xx.h>
#include <pci.h>
#include <i2c.h>
#include <asm/fsl_i2c.h>
#include "vme8349pin.h"

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
 * NOTICE: PCI2 is not supported. There is only one
 * physical PCI slot on the board.
 *
 */
void
pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci1_regions };
	u8 reg8;
	int monarch = 0;

	i2c_set_bus_num(1);
	/* Read the PCI_M66EN jumper setting */
	if ((i2c_read(CONFIG_SYS_I2C_8574_ADDR2, 0, 0, &reg8, 1) == 0) ||
	    (i2c_read(0x38                     , 0, 0, &reg8, 1) == 0)) {
		if (reg8 & 0x40) {
			clk->occr = 0xff000000;	/* 66 MHz PCI */
			printf("PCI:   66MHz\n");
		} else {
			clk->occr = 0xffff0003;	/* 33 MHz PCI */
			printf("PCI:   33MHz\n");
		}
		if (((reg8 & 0x01) == 0) || ((reg8 & 0x02) == 0))
			monarch = 1;
	} else {
		clk->occr = 0xffff0003;	/* 33 MHz PCI */
		printf("PCI:   33MHz (I2C read failed)\n");
	}
	udelay(2000);

	/*
	 * Assert/deassert VME reset
	 */
	clrsetbits_be32(&immr->gpio[1].dat,
			GPIO2_TSI_POWERUP_RESET_N | GPIO2_TSI_PLL_RESET_N,
			GPIO2_VME_RESET_N  | GPIO2_L_RESET_EN_N);
	setbits_be32(&immr->gpio[1].dir, GPIO2_TSI_PLL_RESET_N |
		     GPIO2_TSI_POWERUP_RESET_N |
		     GPIO2_VME_RESET_N |
		     GPIO2_L_RESET_EN_N);
	clrbits_be32(&immr->gpio[1].dir, GPIO2_V_SCON);
	udelay(200);
	setbits_be32(&immr->gpio[1].dat, GPIO2_TSI_PLL_RESET_N);
	udelay(200);
	setbits_be32(&immr->gpio[1].dat, GPIO2_TSI_POWERUP_RESET_N);
	udelay(600000);
	clrbits_be32(&immr->gpio[1].dat, GPIO2_L_RESET_EN_N);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_1G;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_4M;

	udelay(2000);

	if (monarch == 0) {
		mpc83xx_pci_init(1, reg);
	} else {
		/*
		 * Release PCI RST Output signal
		 */
		out_be32(&immr->pci_ctrl[0].gcr, 0);
		udelay(2000);
		out_be32(&immr->pci_ctrl[0].gcr, 1);
	}
}

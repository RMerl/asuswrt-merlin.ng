// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007-2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/fsl_serdes.h>

void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}

void pci_of_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}

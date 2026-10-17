// SPDX-License-Identifier: GPL-2.0+ OR X11
/*
 * Copyright 2019 NXP
 *
 * PCIe Kernel DT fixup of DM U-Boot driver for Freescale PowerPC SoCs
 * Author: Hou Zhiqiang <Zhiqiang.Hou@nxp.com>
 */

#include <common.h>
#ifdef CONFIG_OF_BOARD_SETUP
#include <dm.h>
#include <fdt_support.h>
#include <asm/fsl_pci.h>
#include <linux/libfdt.h>
#include "pcie_fsl.h"

static void ft_fsl_pcie_setup(void *blob, struct fsl_pcie *pcie)
{
	struct pci_controller *hose = dev_get_uclass_priv(pcie->bus);
	fdt_addr_t regs_addr;
	int off;

	regs_addr = dev_read_addr(pcie->bus);
	off = fdt_node_offset_by_compat_reg(blob, FSL_PCIE_COMPAT, regs_addr);
	if (off < 0) {
		printf("%s: Fail to find PCIe node@0x%pa\n",
		       FSL_PCIE_COMPAT, &regs_addr);
		return;
	}

	if (!hose || !pcie->enabled)
		fdt_del_node(blob, off);
	else
		fdt_pci_dma_ranges(blob, off, hose);
}

/* Fixup Kernel DT for PCIe */
void pci_of_setup(void *blob, struct bd_info *bd)
{
	struct fsl_pcie *pcie;

	list_for_each_entry(pcie, &fsl_pcie_list, list)
		ft_fsl_pcie_setup(blob, pcie);
}

#else
void pci_of_setup(void *blob, struct bd_info *bd)
{
}
#endif

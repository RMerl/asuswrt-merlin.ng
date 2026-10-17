/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 *
 * PCIe DT fixup for NXP Layerscape SoCs
 * Author: Wasim Khan <wasim.khan@nxp.com>
 *
 */
#ifndef _PCIE_LAYERSCAPE_FIXUP_COMMON_H_
#define _PCIE_LAYERSCAPE_FIXUP_COMMON_H_

void ft_pci_setup_ls(void *blob, struct bd_info *bd);

#ifdef CONFIG_PCIE_LAYERSCAPE_GEN4
void ft_pci_setup_ls_gen4(void *blob, struct bd_info *bd);
#endif /* CONFIG_PCIE_LAYERSCAPE_GEN4 */
int pcie_next_streamid(int currentid, int id);
int pcie_board_fix_fdt(void *fdt);

#endif //_PCIE_LAYERSCAPE_FIXUP_COMMON_H_

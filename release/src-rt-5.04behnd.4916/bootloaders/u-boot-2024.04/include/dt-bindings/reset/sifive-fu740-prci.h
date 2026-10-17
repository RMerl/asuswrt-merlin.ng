/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2020-2021 Sifive, Inc.
 * Author: Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#ifndef __DT_BINDINGS_RESET_SIFIVE_FU740_PRCI_H
#define __DT_BINDINGS_RESET_SIFIVE_FU740_PRCI_H

/* Reset indexes for use by device tree data and the PRCI driver */
#define PRCI_RST_DDR_CTRL_N		0
#define PRCI_RST_DDR_AXI_N		1
#define PRCI_RST_DDR_AHB_N		2
#define PRCI_RST_DDR_PHY_N		3
#define PRCI_RST_PCIE_POWER_UP_N	4
#define PRCI_RST_GEMGXL_N		5
#define PRCI_RST_CLTX_N			6

#endif

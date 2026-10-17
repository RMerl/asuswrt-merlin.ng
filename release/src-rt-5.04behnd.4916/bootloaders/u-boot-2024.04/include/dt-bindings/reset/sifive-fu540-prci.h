/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Sifive, Inc.
 * Author: Sagar Kadam <sagar.kadam@sifive.com>
 */

#ifndef __DT_BINDINGS_RESET_SIFIVE_FU540_PRCI_H
#define __DT_BINDINGS_RESET_SIFIVE_FU540_PRCI_H

/* Reset indexes for use by device tree data and the PRCI driver */
#define PRCI_RST_DDR_CTRL_N	0
#define PRCI_RST_DDR_AXI_N	1
#define PRCI_RST_DDR_AHB_N	2
#define PRCI_RST_DDR_PHY_N	3
/* bit 4 is reserved bit */
#define PRCI_RST_RSVD_N		4
#define PRCI_RST_GEMGXL_N	5

#endif

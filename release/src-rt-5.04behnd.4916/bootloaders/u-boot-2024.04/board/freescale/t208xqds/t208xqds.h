/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2011-2013 Freescale Semiconductor, Inc.
 */

#ifndef __CORENET_DS_H__
#define __CORENET_DS_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, struct bd_info *bd);

#endif

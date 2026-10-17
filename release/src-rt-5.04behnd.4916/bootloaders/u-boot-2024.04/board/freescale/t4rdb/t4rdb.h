/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __T4RDB_H__
#define __T4RDB_H__

#undef CFG_SYS_NUM_FM1_DTSEC
#undef CFG_SYS_NUM_FM2_DTSEC
#define CFG_SYS_NUM_FM1_DTSEC	4
#define CFG_SYS_NUM_FM2_DTSEC	4

#define CORTINA_FW_ADDR_IFCNOR				0xefe00000
#define CORTINA_FW_ADDR_IFCNOR_ALTBANK		0xebf00000

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, struct bd_info *bd);

#endif

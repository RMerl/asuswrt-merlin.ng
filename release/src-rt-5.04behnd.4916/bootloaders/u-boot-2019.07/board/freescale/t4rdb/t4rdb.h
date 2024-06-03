/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __T4RDB_H__
#define __T4RDB_H__

#undef CONFIG_SYS_NUM_FM1_DTSEC
#undef CONFIG_SYS_NUM_FM2_DTSEC
#define CONFIG_SYS_NUM_FM1_DTSEC	4
#define CONFIG_SYS_NUM_FM2_DTSEC	4

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t *bd);

#endif

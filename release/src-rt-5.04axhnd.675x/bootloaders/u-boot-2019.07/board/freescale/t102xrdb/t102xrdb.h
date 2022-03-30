/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#ifndef __T1024_RDB_H__
#define __T1024_RDB_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t *bd);
#ifdef CONFIG_TARGET_T1023RDB
static u32 t1023rdb_ctrl(u32 ctrl_type);
static void fdt_enable_nor(void *blob);
#endif
#endif

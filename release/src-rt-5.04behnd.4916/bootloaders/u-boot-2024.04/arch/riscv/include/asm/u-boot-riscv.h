/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef _U_BOOT_RISCV_H_
#define _U_BOOT_RISCV_H_	1

/* cpu/.../cpu.c */
int cleanup_before_linux(void);

/* board/.../... */
int board_init(void);
void board_quiesce_devices(void);
int riscv_board_reserved_mem_fixup(void *fdt);
int riscv_fdt_copy_resv_mem_node(const void *src_fdt, void *dest_fdt);

#endif	/* _U_BOOT_RISCV_H_ */

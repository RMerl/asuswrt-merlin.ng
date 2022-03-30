/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/linkage.h>

ENTRY(spin_table_secondary_jump)
.globl spin_table_reserve_begin
spin_table_reserve_begin:
0:	wfe
	ldr	x0, spin_table_cpu_release_addr
	cbz	x0, 0b
	br	x0
.globl spin_table_cpu_release_addr
	.align	3
spin_table_cpu_release_addr:
	.quad	0
.globl spin_table_reserve_end
spin_table_reserve_end:
ENDPROC(spin_table_secondary_jump)

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <asm/assembler.h>

.globl	reset
.type reset, %function
reset:
	W(b)	_main

.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:
	mov	pc, lr

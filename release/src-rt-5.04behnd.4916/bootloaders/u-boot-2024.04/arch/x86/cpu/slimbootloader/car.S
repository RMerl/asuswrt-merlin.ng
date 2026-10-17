/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#include <generated/asm-offsets.h>

.section .text

.globl car_init
car_init:
	/* Get hob pointer parameter from previous stage's stack */
	mov	0x4(%esp), %esi
	jmp	car_init_ret

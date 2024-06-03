/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <config.h>
#include <asm/post.h>

.globl car_init
car_init:
	/* Save the BIST result */
	movl	%eax, %ebp

	post_code(POST_CAR_START)

	/*
	 * Since we know we are running inside emulator,
	 * we can do nothing here for CAR initialization.
	 */

	/* Restore the BIST result */
	movl	%ebp, %eax

	post_code(POST_CAR_CPU_CACHE)
	jmp	car_init_ret

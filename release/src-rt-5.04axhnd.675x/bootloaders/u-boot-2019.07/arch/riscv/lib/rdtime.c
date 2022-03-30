// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Anup Patel <anup@brainfault.org>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * The riscv_get_time() API implementation that is using the
 * standard rdtime instruction.
 */

#include <common.h>

/* Implement the API required by RISC-V timer driver */
int riscv_get_time(u64 *time)
{
#ifdef CONFIG_64BIT
	u64 n;

	__asm__ __volatile__ (
		"rdtime %0"
		: "=r" (n));

	*time = n;
#else
	u32 lo, hi, tmp;

	__asm__ __volatile__ (
		"1:\n"
		"rdtimeh %0\n"
		"rdtime %1\n"
		"rdtimeh %2\n"
		"bne %0, %2, 1b"
		: "=&r" (hi), "=&r" (lo), "=&r" (tmp));

	*time = ((u64)hi << 32) | lo;
#endif

	return 0;
}

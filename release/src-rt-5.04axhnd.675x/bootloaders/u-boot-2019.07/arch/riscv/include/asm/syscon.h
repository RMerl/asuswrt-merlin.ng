/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ASM_SYSCON_H
#define _ASM_SYSCON_H

/*
 * System controllers in a RISC-V system
 */
enum {
	RISCV_NONE,
	RISCV_SYSCON_CLINT,	/* Core Local Interruptor (CLINT) */
	RISCV_SYSCON_PLIC,	/* Platform Level Interrupt Controller (PLIC) */
	RISCV_SYSCON_PLMT,	/* Platform Level Machine Timer (PLMT) */
};

#endif /* _ASM_SYSCON_H */

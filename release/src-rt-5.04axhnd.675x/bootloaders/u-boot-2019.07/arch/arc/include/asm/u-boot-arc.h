/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_U_BOOT_ARC_H__
#define __ASM_ARC_U_BOOT_ARC_H__

int arch_early_init_r(void);

void	board_init_f_r_trampoline(ulong) __attribute__ ((noreturn));
void	board_init_f_r(void) __attribute__ ((noreturn));

#endif	/* __ASM_ARC_U_BOOT_ARC_H__ */

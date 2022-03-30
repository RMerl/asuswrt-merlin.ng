/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef _U_BOOT_NDS32_H_
#define _U_BOOT_NDS32_H_	1

/* for the following variables, see start.S */
extern ulong IRQ_STACK_START;	/* top of IRQ stack */
extern ulong FIQ_STACK_START;	/* top of FIQ stack */

/* cpu/.../cpu.c */
int	cleanup_before_linux(void);

/* board/.../... */
int	board_init(void);

/* cpu/.../interrupt.c */
void	reset_timer_masked(void);

#endif	/* _U_BOOT_NDS32_H_ */

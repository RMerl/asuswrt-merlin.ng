/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 */

#ifndef _U_BOOT_ARM_H_
#define _U_BOOT_ARM_H_	1

#ifndef __ASSEMBLY__

/* for the following variables, see start.S */
extern ulong IRQ_STACK_START;	/* top of IRQ stack */
extern ulong FIQ_STACK_START;	/* top of FIQ stack */
extern ulong _datarel_start_ofs;
extern ulong _datarelrolocal_start_ofs;
extern ulong _datarellocal_start_ofs;
extern ulong _datarelro_start_ofs;
extern ulong IRQ_STACK_START_IN;	/* 8 bytes in IRQ stack */

void s_init(void);

/* cpu/.../cpu.c */
int	cleanup_before_linux(void);

/* Set up ARMv7 MMU, caches and TLBs */
void	cpu_init_cp15(void);

/* cpu/.../arch/cpu.c */
int	arch_misc_init(void);

/* board/.../... */
int	board_init(void);

/* calls to c from vectors.S */
struct pt_regs;

void bad_mode(void);
void do_undefined_instruction(struct pt_regs *pt_regs);
void do_software_interrupt(struct pt_regs *pt_regs);
void do_prefetch_abort(struct pt_regs *pt_regs);
void do_data_abort(struct pt_regs *pt_regs);
void do_not_used(struct pt_regs *pt_regs);
void do_fiq(struct pt_regs *pt_regs);
void do_irq(struct pt_regs *pt_regs);

void reset_misc(void);

#endif /* __ASSEMBLY__ */

#endif	/* _U_BOOT_ARM_H_ */

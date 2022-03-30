// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <common.h>

/*
 * Upon exception entry ARMv7-M processors automatically save stack
 * frames containing some registers. For simplicity initial
 * implementation uses only this auto-saved stack frame.
 * This does not contain complete register set dump,
 * only R0-R3, R12, LR, PC and xPSR are saved.
 */

struct autosave_regs {
	long uregs[8];
};

#define ARM_XPSR	uregs[7]
#define ARM_PC		uregs[6]
#define ARM_LR		uregs[5]
#define ARM_R12		uregs[4]
#define ARM_R3		uregs[3]
#define ARM_R2		uregs[2]
#define ARM_R1		uregs[1]
#define ARM_R0		uregs[0]

int interrupt_init(void)
{
	return 0;
}

void enable_interrupts(void)
{
	return;
}

int disable_interrupts(void)
{
	return 0;
}

void dump_regs(struct autosave_regs *regs)
{
	printf("pc : %08lx    lr : %08lx    xPSR : %08lx\n",
	       regs->ARM_PC, regs->ARM_LR, regs->ARM_XPSR);
	printf("r12 : %08lx   r3 : %08lx    r2 : %08lx\n"
		"r1 : %08lx    r0 : %08lx\n",
		regs->ARM_R12, regs->ARM_R3, regs->ARM_R2,
		regs->ARM_R1, regs->ARM_R0);
}

void bad_mode(void)
{
	panic("Resetting CPU ...\n");
	reset_cpu(0);
}

void do_hard_fault(struct autosave_regs *autosave_regs)
{
	printf("Hard fault\n");
	dump_regs(autosave_regs);
	bad_mode();
}

void do_mm_fault(struct autosave_regs *autosave_regs)
{
	printf("Memory management fault\n");
	dump_regs(autosave_regs);
	bad_mode();
}

void do_bus_fault(struct autosave_regs *autosave_regs)
{
	printf("Bus fault\n");
	dump_regs(autosave_regs);
	bad_mode();
}

void do_usage_fault(struct autosave_regs *autosave_regs)
{
	printf("Usage fault\n");
	dump_regs(autosave_regs);
	bad_mode();
}

void do_invalid_entry(struct autosave_regs *autosave_regs)
{
	printf("Exception\n");
	dump_regs(autosave_regs);
	bad_mode();
}

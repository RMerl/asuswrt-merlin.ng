// SPDX-License-Identifier: GPL-2.0+
/*
 * linux/arch/powerpc/kernel/traps.c
 *
 * Copyright (C) 1995-1996  Gary Thomas (gdt@linuxppc.org)
 *
 * Modified by Cort Dougan (cort@cs.nmt.edu)
 * and Paul Mackerras (paulus@cs.anu.edu.au)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * This file handles the architecture-dependent parts of hardware exceptions
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

/* THIS NEEDS CHANGING to use the board info structure.
*/
#define END_OF_MEM	0x02000000

/*
 * Trap & Exception support
 */

static void print_backtrace(unsigned long *sp)
{
	int cnt = 0;
	unsigned long i;

	printf("Call backtrace: ");
	while (sp) {
		if ((uint)sp > END_OF_MEM)
			break;

		i = sp[1];
		if (cnt++ % 7 == 0)
			printf("\n");
		printf("%08lX ", i);
		if (cnt > 32)
			break;
		sp = (unsigned long *)*sp;
	}
	printf("\n");
}

static void show_regs(struct pt_regs *regs)
{
	int i;

	printf("NIP: %08lX XER: %08lX LR: %08lX REGS: %p TRAP: %04lx DAR: %08lX\n",
	       regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
	printf("MSR: %08lx EE: %01x PR: %01x FP: %01x ME: %01x IR/DR: %01x%01x\n",
	       regs->msr, regs->msr & MSR_EE ? 1 : 0,
	       regs->msr & MSR_PR ? 1 : 0, regs->msr & MSR_FP ? 1 : 0,
	       regs->msr & MSR_ME ? 1 : 0, regs->msr & MSR_IR ? 1 : 0,
	       regs->msr & MSR_DR ? 1 : 0);

	printf("\n");
	for (i = 0;  i < 32;  i++) {
		if ((i % 8) == 0)
			printf("GPR%02d: ", i);

		printf("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7)
			printf("\n");
	}
}


static void _exception(int signr, struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Exception in kernel pc %lx signal %d", regs->nip, signr);
}

void MachineCheckException(struct pt_regs *regs)
{
	unsigned long fixup = search_exception_table(regs->nip);

	/* Probing PCI using config cycles cause this exception
	 * when a device is not present.  Catch it and return to
	 * the PCI exception handler.
	 */
	if (fixup != 0) {
		regs->nip = fixup;
		return;
	}

	printf("Machine check in kernel mode.\n");
	printf("Caused by (from msr): ");
	printf("regs %p ", regs);
	switch (regs->msr & 0x000F0000) {
	case (0x80000000 >> 12):
		printf("Machine check signal - probably due to mm fault\n"
			"with mmu off\n");
		break;
	case (0x80000000 >> 13):
		printf("Transfer error ack signal\n");
		break;
	case (0x80000000 >> 14):
		printf("Data parity signal\n");
		break;
	case (0x80000000 >> 15):
		printf("Address parity signal\n");
		break;
	default:
		printf("Unknown values in msr\n");
	}
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("machine check");
}

void AlignmentException(struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Alignment Exception");
}

void ProgramCheckException(struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Program Check Exception");
}

void SoftEmuException(struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Software Emulation Exception");
}


void UnknownException(struct pt_regs *regs)
{
	printf("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
	       regs->nip, regs->msr, regs->trap);
	_exception(0, regs);
}

void DebugException(struct pt_regs *regs)
{
	printf("Debugger trap at @ %lx\n", regs->nip);
	show_regs(regs);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * linux/arch/powerpc/kernel/traps.c
 *
 * Copyright 2007 Freescale Semiconductor.
 * Copyright (C) 2003 Motorola
 * Modified by Xianghua Xiao(x.xiao@motorola.com)
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
#include <kgdb.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

/*
 * End of addressable memory.  This may be less than the actual
 * amount of memory on the system if we're unable to keep all
 * the memory mapped in.
 */
#define END_OF_MEM (gd->bd->bi_memstart + get_effective_memsize())

static __inline__ void set_tsr(unsigned long val)
{
	asm volatile("mtspr 0x150, %0" : : "r" (val));
}

static __inline__ unsigned long get_esr(void)
{
	unsigned long val;
	asm volatile("mfspr %0, 0x03e" : "=r" (val) :);
	return val;
}

#define ESR_MCI 0x80000000
#define ESR_PIL 0x08000000
#define ESR_PPR 0x04000000
#define ESR_PTR 0x02000000
#define ESR_DST 0x00800000
#define ESR_DIZ 0x00400000
#define ESR_U0F 0x00008000

#if defined(CONFIG_CMD_BEDBUG)
extern void do_bedbug_breakpoint(struct pt_regs *);
#endif

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
		if (cnt > 32) break;
		sp = (unsigned long *)*sp;
	}
	printf("\n");
}

void show_regs(struct pt_regs *regs)
{
	int i;

	printf("NIP: %08lX XER: %08lX LR: %08lX REGS: %p TRAP: %04lx DAR: %08lX\n",
	       regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
	printf("MSR: %08lx EE: %01x PR: %01x FP: %01x ME: %01x IR/DR: %01x%01x\n",
	       regs->msr, regs->msr&MSR_EE ? 1 : 0, regs->msr&MSR_PR ? 1 : 0,
	       regs->msr & MSR_FP ? 1 : 0,regs->msr&MSR_ME ? 1 : 0,
	       regs->msr&MSR_IR ? 1 : 0,
	       regs->msr&MSR_DR ? 1 : 0);

	printf("\n");
	for (i = 0;  i < 32;  i++) {
		if ((i % 8) == 0)
		{
			printf("GPR%02d: ", i);
		}

		printf("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7)
		{
			printf("\n");
		}
	}
}


static void _exception(int signr, struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Exception in kernel pc %lx signal %d",regs->nip,signr);
}

void CritcalInputException(struct pt_regs *regs)
{
	panic("Critical Input Exception");
}

int machinecheck_count = 0;
int machinecheck_error = 0;
void MachineCheckException(struct pt_regs *regs)
{
	unsigned long fixup;
	unsigned int mcsr, mcsrr0, mcsrr1, mcar;

	/* Probing PCI using config cycles cause this exception
	 * when a device is not present.  Catch it and return to
	 * the PCI exception handler.
	 */
	if ((fixup = search_exception_table(regs->nip)) != 0) {
		regs->nip = fixup;
		return;
	}

	mcsrr0 = mfspr(SPRN_MCSRR0);
	mcsrr1 = mfspr(SPRN_MCSRR1);
	mcsr = mfspr(SPRN_MCSR);
	mcar = mfspr(SPRN_MCAR);

	machinecheck_count++;
	machinecheck_error=1;

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	printf("Machine check in kernel mode.\n");
	printf("Caused by (from mcsr): ");
	printf("mcsr = 0x%08x\n", mcsr);
	if (mcsr & 0x80000000)
		printf("Machine check input pin\n");
	if (mcsr & 0x40000000)
		printf("Instruction cache parity error\n");
	if (mcsr & 0x20000000)
		printf("Data cache push parity error\n");
	if (mcsr & 0x10000000)
		printf("Data cache parity error\n");
	if (mcsr & 0x00000080)
		printf("Bus instruction address error\n");
	if (mcsr & 0x00000040)
		printf("Bus Read address error\n");
	if (mcsr & 0x00000020)
		printf("Bus Write address error\n");
	if (mcsr & 0x00000010)
		printf("Bus Instruction data bus error\n");
	if (mcsr & 0x00000008)
		printf("Bus Read data bus error\n");
	if (mcsr & 0x00000004)
		printf("Bus Write bus error\n");
	if (mcsr & 0x00000002)
		printf("Bus Instruction parity error\n");
	if (mcsr & 0x00000001)
		printf("Bus Read parity error\n");

	show_regs(regs);
	printf("MCSR=0x%08x \tMCSRR0=0x%08x \nMCSRR1=0x%08x \tMCAR=0x%08x\n",
	       mcsr, mcsrr0, mcsrr1, mcar);
	print_backtrace((unsigned long *)regs->gpr[1]);
	if (machinecheck_count > 10) {
		panic("machine check count too high\n");
	}

	if (machinecheck_count > 1) {
		regs->nip += 4; /* skip offending instruction */
		printf("Skipping current instr, Returning to 0x%08lx\n",
		       regs->nip);
	} else {
		printf("Returning back to 0x%08lx\n",regs->nip);
	}
}

void AlignmentException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Alignment Exception");
}

void ProgramCheckException(struct pt_regs *regs)
{
	long esr_val;

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	show_regs(regs);

	esr_val = get_esr();
	if( esr_val & ESR_PIL )
		printf( "** Illegal Instruction **\n" );
	else if( esr_val & ESR_PPR )
		printf( "** Privileged Instruction **\n" );
	else if( esr_val & ESR_PTR )
		printf( "** Trap Instruction **\n" );

	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Program Check Exception");
}

void PITException(struct pt_regs *regs)
{
	/*
	 * Reset PIT interrupt
	 */
	set_tsr(0x0c000000);

	/*
	 * Call timer_interrupt routine in interrupts.c
	 */
	timer_interrupt(NULL);
}

void UnknownException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	printf("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
	       regs->nip, regs->msr, regs->trap);
	_exception(0, regs);
}

void ExtIntException(struct pt_regs *regs)
{
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);

	uint vect;

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	printf("External Interrupt Exception at PC: %lx, SR: %lx, vector=%lx",
	       regs->nip, regs->msr, regs->trap);
	vect = pic->iack0;
	printf(" irq IACK0@%05x=%d\n",(int)&pic->iack0,vect);
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
}

void DebugException(struct pt_regs *regs)
{
	printf("Debugger trap at @ %lx\n", regs->nip );
	show_regs(regs);
#if defined(CONFIG_CMD_BEDBUG)
	do_bedbug_breakpoint( regs );
#endif
}

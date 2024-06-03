// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <asm/arcregs.h>
#include <asm/ptrace.h>

/* Bit values in STATUS32 */
#define E1_MASK		(1 << 1)	/* Level 1 interrupts enable */
#define E2_MASK		(1 << 2)	/* Level 2 interrupts enable */

int interrupt_init(void)
{
	return 0;
}

/*
 * returns true if interrupts had been enabled before we disabled them
 */
int disable_interrupts(void)
{
	int status = read_aux_reg(ARC_AUX_STATUS32);
	int state = (status & (E1_MASK | E2_MASK)) ? 1 : 0;

	status &= ~(E1_MASK | E2_MASK);
	/* STATUS32 register is updated indirectly with "FLAG" instruction */
	__asm__("flag %0" : : "r" (status));
	return state;
}

void enable_interrupts(void)
{
	unsigned int status = read_aux_reg(ARC_AUX_STATUS32);

	status |= E1_MASK | E2_MASK;
	/* STATUS32 register is updated indirectly with "FLAG" instruction */
	__asm__("flag %0" : : "r" (status));
}

static void print_reg_file(long *reg_rev, int start_num)
{
	unsigned int i;

	/* Print 3 registers per line */
	for (i = start_num; i < start_num + 25; i++) {
		printf("r%02u: 0x%08lx\t", i, (unsigned long)*reg_rev);
		if (((i + 1) % 3) == 0)
			printf("\n");

		/* Because pt_regs has registers reversed */
		reg_rev--;
	}

	/* Add new-line if none was inserted in the end of loop above */
	if (((i + 1) % 3) != 0)
		printf("\n");
}

void show_regs(struct pt_regs *regs)
{
	printf("ECR:\t0x%08lx\n", regs->ecr);
	printf("RET:\t0x%08lx\nBLINK:\t0x%08lx\nSTAT32:\t0x%08lx\n",
	       regs->ret, regs->blink, regs->status32);
	printf("GP: 0x%08lx\t r25: 0x%08lx\t\n", regs->r26, regs->r25);
	printf("BTA: 0x%08lx\t SP: 0x%08lx\t FP: 0x%08lx\n", regs->bta,
	       regs->sp, regs->fp);
	printf("LPS: 0x%08lx\tLPE: 0x%08lx\tLPC: 0x%08lx\n", regs->lp_start,
	       regs->lp_end, regs->lp_count);

	print_reg_file(&(regs->r0), 0);
}

void bad_mode(struct pt_regs *regs)
{
	if (regs)
		show_regs(regs);

	panic("Resetting CPU ...\n");
}

void do_memory_error(unsigned long address, struct pt_regs *regs)
{
	printf("Memory error exception @ 0x%lx\n", address);
	bad_mode(regs);
}

void do_instruction_error(unsigned long address, struct pt_regs *regs)
{
	printf("Instruction error exception @ 0x%lx\n", address);
	bad_mode(regs);
}

void do_machine_check_fault(unsigned long address, struct pt_regs *regs)
{
	printf("Machine check exception @ 0x%lx\n", address);
	bad_mode(regs);
}

void do_interrupt_handler(void)
{
	printf("Interrupt fired\n");
	bad_mode(0);
}

void do_itlb_miss(struct pt_regs *regs)
{
	printf("I TLB miss exception\n");
	bad_mode(regs);
}

void do_dtlb_miss(struct pt_regs *regs)
{
	printf("D TLB miss exception\n");
	bad_mode(regs);
}

void do_tlb_prot_violation(unsigned long address, struct pt_regs *regs)
{
	printf("TLB protection violation or misaligned access @ 0x%lx\n",
	       address);
	bad_mode(regs);
}

void do_privilege_violation(struct pt_regs *regs)
{
	printf("Privilege violation exception\n");
	bad_mode(regs);
}

void do_trap(struct pt_regs *regs)
{
	printf("Trap exception\n");
	bad_mode(regs);
}

void do_extension(struct pt_regs *regs)
{
	printf("Extension instruction exception\n");
	bad_mode(regs);
}

#ifdef CONFIG_ISA_ARCV2
void do_swi(struct pt_regs *regs)
{
	printf("Software Interrupt exception\n");
	bad_mode(regs);
}

void do_divzero(unsigned long address, struct pt_regs *regs)
{
	printf("Division by zero exception @ 0x%lx\n", address);
	bad_mode(regs);
}

void do_dcerror(struct pt_regs *regs)
{
	printf("Data cache consistency error exception\n");
	bad_mode(regs);
}

void do_maligned(unsigned long address, struct pt_regs *regs)
{
	printf("Misaligned data access exception @ 0x%lx\n", address);
	bad_mode(regs);
}
#endif

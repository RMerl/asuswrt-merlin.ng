// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-17 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * Copyright (C) 2019 Sean Anderson <seanga2@gmail.com>
 */

#include <linux/compat.h>
#include <efi_loader.h>
#include <hang.h>
#include <interrupt.h>
#include <irq_func.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/encoding.h>
#include <semihosting.h>

DECLARE_GLOBAL_DATA_PTR;

static struct resume_data *resume;

void set_resume(struct resume_data *data)
{
	resume = data;
}

static void show_efi_loaded_images(uintptr_t epc)
{
	efi_print_image_infos((void *)epc);
}

static void show_regs(struct pt_regs *regs)
{
#ifdef CONFIG_SHOW_REGS
	printf("\nSP:  " REG_FMT " GP:  " REG_FMT " TP:  " REG_FMT "\n",
	       regs->sp, regs->gp, regs->tp);
	printf("T0:  " REG_FMT " T1:  " REG_FMT " T2:  " REG_FMT "\n",
	       regs->t0, regs->t1, regs->t2);
	printf("S0:  " REG_FMT " S1:  " REG_FMT " A0:  " REG_FMT "\n",
	       regs->s0, regs->s1, regs->a0);
	printf("A1:  " REG_FMT " A2:  " REG_FMT " A3:  " REG_FMT "\n",
	       regs->a1, regs->a2, regs->a3);
	printf("A4:  " REG_FMT " A5:  " REG_FMT " A6:  " REG_FMT "\n",
	       regs->a4, regs->a5, regs->a6);
	printf("A7:  " REG_FMT " S2:  " REG_FMT " S3:  " REG_FMT "\n",
	       regs->a7, regs->s2, regs->s3);
	printf("S4:  " REG_FMT " S5:  " REG_FMT " S6:  " REG_FMT "\n",
	       regs->s4, regs->s5, regs->s6);
	printf("S7:  " REG_FMT " S8:  " REG_FMT " S9:  " REG_FMT "\n",
	       regs->s7, regs->s8, regs->s9);
	printf("S10: " REG_FMT " S11: " REG_FMT " T3:  " REG_FMT "\n",
	       regs->s10, regs->s11, regs->t3);
	printf("T4:  " REG_FMT " T5:  " REG_FMT " T6:  " REG_FMT "\n",
	       regs->t4, regs->t5, regs->t6);
#endif
}

/**
 * instr_len() - get instruction length
 *
 * @i:		low 16 bits of the instruction
 * Return:	number of u16 in instruction
 */
static int instr_len(u16 i)
{
	if ((i & 0x03) != 0x03)
		return 1;
	/* Instructions with more than 32 bits are not yet specified */
	return 2;
}

/**
 * show_code() - display code leading to exception
 *
 * @epc:	program counter
 */
static void show_code(ulong epc)
{
	u16 *pos = (u16 *)(epc & ~1UL);
	int i, len = instr_len(*pos);

	printf("\nCode: ");
	for (i = -8; i; ++i)
		printf("%04x ", pos[i]);
	printf("(");
	for (i = 0; i < len; ++i)
		printf("%04x%s", pos[i], i + 1 == len ? ")\n" : " ");
}

static void _exit_trap(ulong code, ulong epc, ulong tval, struct pt_regs *regs)
{
	static const char * const exception_code[] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode",
		"Environment call from S-mode",
		"Reserved",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault",
	};

	if (resume) {
		resume->code = code;
		longjmp(resume->jump, 1);
	}

	if (code < ARRAY_SIZE(exception_code))
		printf("Unhandled exception: %s\n", exception_code[code]);
	else
		printf("Unhandled exception code: %ld\n", code);

	printf("EPC: " REG_FMT " RA: " REG_FMT " TVAL: " REG_FMT "\n",
	       epc, regs->ra, tval);
	/* Print relocation adjustments, but only if gd is initialized */
	if (gd && gd->flags & GD_FLG_RELOC)
		printf("EPC: " REG_FMT " RA: " REG_FMT " reloc adjusted\n",
		       epc - gd->reloc_off, regs->ra - gd->reloc_off);

	show_regs(regs);
	show_code(epc);
	show_efi_loaded_images(epc);
	panic("\n");
}

int interrupt_init(void)
{
	return 0;
}

/*
 * enable interrupts
 */
void enable_interrupts(void)
{
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
	return 0;
}

ulong handle_trap(ulong cause, ulong epc, ulong tval, struct pt_regs *regs)
{
	ulong is_irq, irq;

	/* An UEFI application may have changed gd. Restore U-Boot's gd. */
	efi_restore_gd();

	if (cause == CAUSE_BREAKPOINT &&
	    CONFIG_IS_ENABLED(SEMIHOSTING_FALLBACK)) {
		ulong pre_addr = epc - 4, post_addr = epc + 4;

		/* Check for prior and post addresses to be in same page. */
		if ((pre_addr & ~(PAGE_SIZE - 1)) ==
			(post_addr & ~(PAGE_SIZE - 1))) {
			u32 pre = *(u32 *)pre_addr;
			u32 post = *(u32 *)post_addr;

			/* Check for semihosting, i.e.:
			 * slli    zero,zero,0x1f
			 * ebreak
			 * srai    zero,zero,0x7
			 */
			if (pre == 0x01f01013 && post == 0x40705013) {
				disable_semihosting();
				epc += 4;
				return epc;
			}
		}
	}

	is_irq = (cause & MCAUSE_INT);
	irq = (cause & ~MCAUSE_INT);

	if (is_irq) {
		switch (irq) {
		case IRQ_M_EXT:
		case IRQ_S_EXT:
			external_interrupt(0);	/* handle external interrupt */
			break;
		case IRQ_M_TIMER:
		case IRQ_S_TIMER:
			timer_interrupt(0);	/* handle timer interrupt */
			break;
		default:
			_exit_trap(cause, epc, tval, regs);
			break;
		};
	} else {
		_exit_trap(cause, epc, tval, regs);
	}

	return epc;
}

/*
 *Entry Point for PLIC Interrupt Handler
 */
__attribute__((weak)) void external_interrupt(struct pt_regs *regs)
{
}

__attribute__((weak)) void timer_interrupt(struct pt_regs *regs)
{
}

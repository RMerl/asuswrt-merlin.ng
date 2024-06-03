// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 */

#include <common.h>
#include <efi_loader.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/u-boot-arm.h>

DECLARE_GLOBAL_DATA_PTR;

int interrupt_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
	IRQ_STACK_START_IN = gd->irq_sp + 8;

	return 0;
}

void enable_interrupts (void)
{
	return;
}
int disable_interrupts (void)
{
	return 0;
}

void bad_mode (void)
{
	panic ("Resetting CPU ...\n");
	reset_cpu (0);
}

static void show_efi_loaded_images(struct pt_regs *regs)
{
	efi_print_image_infos((void *)instruction_pointer(regs));
}

static void dump_instr(struct pt_regs *regs)
{
	unsigned long addr = instruction_pointer(regs);
	const int thumb = thumb_mode(regs);
	const int width = thumb ? 4 : 8;
	int i;

	if (thumb)
		addr &= ~1L;
	else
		addr &= ~3L;
	printf("Code: ");
	for (i = -4; i < 1 + !!thumb; i++) {
		unsigned int val;

		if (thumb)
			val = ((u16 *)addr)[i];
		else
			val = ((u32 *)addr)[i];
		printf(i == 0 ? "(%0*x) " : "%0*x ", width, val);
	}
	printf("\n");
}

void show_regs (struct pt_regs *regs)
{
	unsigned long __maybe_unused flags;
	const char __maybe_unused *processor_modes[] = {
	"USER_26",	"FIQ_26",	"IRQ_26",	"SVC_26",
	"UK4_26",	"UK5_26",	"UK6_26",	"UK7_26",
	"UK8_26",	"UK9_26",	"UK10_26",	"UK11_26",
	"UK12_26",	"UK13_26",	"UK14_26",	"UK15_26",
	"USER_32",	"FIQ_32",	"IRQ_32",	"SVC_32",
	"UK4_32",	"UK5_32",	"UK6_32",	"ABT_32",
	"UK8_32",	"UK9_32",	"HYP_32",	"UND_32",
	"UK12_32",	"UK13_32",	"UK14_32",	"SYS_32",
	};

	flags = condition_codes (regs);

	printf("pc : [<%08lx>]	   lr : [<%08lx>]\n",
	       instruction_pointer(regs), regs->ARM_lr);
	if (gd->flags & GD_FLG_RELOC) {
		printf("reloc pc : [<%08lx>]	   lr : [<%08lx>]\n",
		       instruction_pointer(regs) - gd->reloc_off,
		       regs->ARM_lr - gd->reloc_off);
	}
	printf("sp : %08lx  ip : %08lx	 fp : %08lx\n",
	       regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf ("r10: %08lx  r9 : %08lx	 r8 : %08lx\n",
		regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf ("r7 : %08lx  r6 : %08lx	 r5 : %08lx  r4 : %08lx\n",
		regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf ("r3 : %08lx  r2 : %08lx	 r1 : %08lx  r0 : %08lx\n",
		regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	printf ("Flags: %c%c%c%c",
		flags & CC_N_BIT ? 'N' : 'n',
		flags & CC_Z_BIT ? 'Z' : 'z',
		flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	printf ("  IRQs %s  FIQs %s  Mode %s%s\n",
		interrupts_enabled (regs) ? "on" : "off",
		fast_interrupts_enabled (regs) ? "on" : "off",
		processor_modes[processor_mode (regs)],
		thumb_mode (regs) ? " (T)" : "");
	dump_instr(regs);
}

/* fixup PC to point to the instruction leading to the exception */
static inline void fixup_pc(struct pt_regs *regs, int offset)
{
	uint32_t pc = instruction_pointer(regs) + offset;
	regs->ARM_pc = pc | (regs->ARM_pc & PCMASK);
}

void do_undefined_instruction (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("undefined instruction\n");
	fixup_pc(pt_regs, -4);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_software_interrupt (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("software interrupt\n");
	fixup_pc(pt_regs, -4);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_prefetch_abort (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("prefetch abort\n");
	fixup_pc(pt_regs, -8);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_data_abort (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("data abort\n");
	fixup_pc(pt_regs, -8);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_not_used (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("not used\n");
	fixup_pc(pt_regs, -8);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_fiq (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("fast interrupt request\n");
	fixup_pc(pt_regs, -8);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

void do_irq (struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf ("interrupt request\n");
	fixup_pc(pt_regs, -8);
	show_regs (pt_regs);
	show_efi_loaded_images(pt_regs);
	bad_mode ();
}

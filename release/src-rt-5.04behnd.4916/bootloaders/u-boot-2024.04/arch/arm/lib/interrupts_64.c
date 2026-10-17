// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <asm/esr.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <irq_func.h>
#include <linux/compiler.h>
#include <efi_loader.h>
#include <semihosting.h>

DECLARE_GLOBAL_DATA_PTR;

int interrupt_init(void)
{
	enable_interrupts();

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

static void show_efi_loaded_images(struct pt_regs *regs)
{
	efi_print_image_infos((void *)regs->elr);
}

static void dump_far(unsigned long esr)
{
	unsigned long el, far;

	switch ((esr >> 26) & 0b111111) {
	case 0x20:
	case 0x21:
	case 0x24:
	case 0x25:
	case 0x22:
	case 0x34:
	case 0x35:
		break;
	default:
		return;
	}

	asm("mrs	%0, CurrentEl": "=r" (el));

	switch (el >> 2) {
	case 1:
		asm("mrs	%0, FAR_EL1": "=r" (far));
		break;
	case 2:
		asm("mrs	%0, FAR_EL2": "=r" (far));
		break;
	default:
		/* don't print anything to make output pretty */
		return;
	}

	printf(", far 0x%lx", far);
}

static void dump_instr(struct pt_regs *regs)
{
	u32 *addr = (u32 *)(regs->elr & ~3UL);
	int i;

	printf("Code: ");
	for (i = -4; i < 1; i++)
		printf(i == 0 ? "(%08x) " : "%08x ", addr[i]);
	printf("\n");
}

void show_regs(struct pt_regs *regs)
{
	int i;

	if (gd->flags & GD_FLG_RELOC)
		printf("elr: %016lx lr : %016lx (reloc)\n",
		       regs->elr - gd->reloc_off,
		       regs->regs[30] - gd->reloc_off);
	printf("elr: %016lx lr : %016lx\n", regs->elr, regs->regs[30]);

	for (i = 0; i < 29; i += 2)
		printf("x%-2d: %016lx x%-2d: %016lx\n",
		       i, regs->regs[i], i+1, regs->regs[i+1]);
	printf("\n");
	dump_instr(regs);
}

/*
 * Try to "emulate" a semihosting call in the event that we don't have a
 * debugger attached.
 */
static bool smh_emulate_trap(struct pt_regs *regs)
{
	int size;

	if (ESR_ELx_EC(regs->esr) != ESR_ELx_EC_UNKNOWN)
		return false;

	if (regs->spsr & PSR_MODE32_BIT) {
		if (regs->spsr & PSR_AA32_T_BIT) {
			u16 *insn = (u16 *)ALIGN_DOWN(regs->elr, 2);

			if (*insn != SMH_T32_SVC && *insn != SMH_T32_HLT)
				return false;
			size = 2;
		} else {
			u32 *insn = (u32 *)ALIGN_DOWN(regs->elr, 4);

			if (*insn != SMH_A32_SVC && *insn != SMH_A32_HLT)
				return false;
			size = 4;
		}
	} else {
		u32 *insn = (u32 *)ALIGN_DOWN(regs->elr, 4);

		if (*insn != SMH_A64_HLT)
			return false;
		size = 4;
	}

	/* Avoid future semihosting calls */
	disable_semihosting();

	/* Just pretend the call failed */
	regs->regs[0] = -1;
	regs->elr += size;
	return true;
}

/*
 * do_bad_sync handles the impossible case in the Synchronous Abort vector.
 */
void do_bad_sync(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("Bad mode in \"Synchronous Abort\" handler, esr 0x%08lx\n",
	       pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("Bad mode in \"Irq\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("Bad mode in \"Fiq\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("Bad mode in \"Error\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct pt_regs *pt_regs)
{
	if (CONFIG_IS_ENABLED(SEMIHOSTING_FALLBACK) &&
	    smh_emulate_trap(pt_regs))
		return;
	efi_restore_gd();
	printf("\"Synchronous Abort\" handler, esr 0x%08lx", pt_regs->esr);
	dump_far(pt_regs->esr);
	printf("\n");
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_irq handles the Irq exception.
 */
void do_irq(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("\"Irq\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("\"Fiq\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct pt_regs *pt_regs)
{
	efi_restore_gd();
	printf("\"Error\" handler, esr 0x%08lx\n", pt_regs->esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

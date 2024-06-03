// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <linux/compiler.h>
#include <efi_loader.h>

DECLARE_GLOBAL_DATA_PTR;

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

static void show_efi_loaded_images(struct pt_regs *regs)
{
	efi_print_image_infos((void *)regs->elr);
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
}

/*
 * do_bad_sync handles the impossible case in the Synchronous Abort vector.
 */
void do_bad_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_irq handles the Irq exception.
 */
void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Fiq\" handler, esr 0x%08x\n", esr);
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
void __weak do_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

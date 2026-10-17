// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 1995-1996  Gary Thomas (gdt@linuxppc.org)
 */

/*
 * This file handles the architecture-dependent parts of hardware
 * exceptions
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <command.h>
#include <kgdb.h>
#include <asm/processor.h>
#include <asm/mpc8349_pci.h>

DECLARE_GLOBAL_DATA_PTR;

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

#define END_OF_MEM	(gd->ram_base + gd->ram_size)

/*
 * Trap & Exception support
 */

static void print_backtrace(unsigned long *sp)
{
	int cnt = 0;
	unsigned long i;

	puts ("Call backtrace: ");
	while (sp) {
		if ((uint)sp > END_OF_MEM)
			break;

		i = sp[1];
		if (cnt++ % 7 == 0)
			putc ('\n');
		printf("%08lX ", i);
		if (cnt > 32) break;
		sp = (unsigned long *)*sp;
	}
	putc ('\n');
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

	putc ('\n');
	for (i = 0;  i < 32;  i++) {
		if ((i % 8) == 0) {
			printf("GPR%02d: ", i);
		}

		printf("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7) {
			putc ('\n');
		}
	}
}


static void _exception(int signr, struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Exception in kernel pc %lx signal %d",regs->nip,signr);
}

#ifdef CONFIG_PCI
void dump_pci (void)
{
/*
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	printf ("PCI: err status %x err mask %x err ctrl %x\n",
		le32_to_cpu (immap->im_pci.pci_esr),
		le32_to_cpu (immap->im_pci.pci_emr),
		le32_to_cpu (immap->im_pci.pci_ecr));
	printf ("     error address %x error data %x ctrl %x\n",
		le32_to_cpu (immap->im_pci.pci_eacr),
		le32_to_cpu (immap->im_pci.pci_edcr),
		le32_to_cpu (immap->im_pci.pci_eccr));
*/
}
#endif

void MachineCheckException(struct pt_regs *regs)
{
	unsigned long fixup;

	/* Probing PCI using config cycles cause this exception
	 * when a device is not present.  Catch it and return to
	 * the PCI exception handler.
	 */
#ifdef CONFIG_PCI
#if 0
	volatile immap_t *immap  = (immap_t *)CONFIG_SYS_IMMR;
#ifdef DEBUG
	dump_pci();
#endif
	/* clear the error in the error status register */
	if(immap->im_pci.pci_esr & cpu_to_le32(PCI_ERROR_PCI_NO_RSP)) {
		immap->im_pci.pci_esr = cpu_to_le32(PCI_ERROR_PCI_NO_RSP);
		return;
	}
#endif
#endif /* CONFIG_PCI */
	if ((fixup = search_exception_table(regs->nip)) != 0) {
		regs->nip = fixup;
		return;
	}

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	puts ("Machine check in kernel mode.\n"
		"Caused by (from msr): ");
	printf("regs %p ",regs);
	switch( regs->msr & 0x000F0000) {
	case (0x80000000>>12):
		puts ("Machine check signal - probably due to mm fault\n"
			"with mmu off\n");
		break;
	case (0x80000000>>13):
		puts ("Transfer error ack signal\n");
		break;
	case (0x80000000>>14):
		puts ("Data parity signal\n");
		break;
	case (0x80000000>>15):
		puts ("Address parity signal\n");
		break;
	default:
		puts ("Unknown values in msr\n");
	}
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
#ifdef CONFIG_PCI
	dump_pci();
#endif
	panic("machine check");
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
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Program Check Exception");
}

void SoftEmuException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Software Emulation Exception");
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

void DebugException(struct pt_regs *regs)
{
	printf("Debugger trap at @ %lx\n", regs->nip );
	show_regs(regs);
}

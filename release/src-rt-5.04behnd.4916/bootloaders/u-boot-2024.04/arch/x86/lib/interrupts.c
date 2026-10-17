// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, <daniel@gaisler.com>
 *
 * (C) Copyright 2006
 * Detlev Zundel, DENX Software Engineering, <dzu@denx.de>
 *
 * (C) Copyright -2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2001
 * Josh Huber, Mission Critical Linux, Inc, <huber@mclx.com>
 */

/*
 * This file contains the high-level API for the interrupt sub-system
 * of the x86 port of U-Boot. Most of the functionality has been
 * shamelessly stolen from the leon2 / leon3 ports of U-Boot.
 * Daniel Hellstrom, Detlev Zundel, Wolfgang Denk and Josh Huber are
 * credited for the corresponding work on those ports. The original
 * interrupt handling routines for the x86 port were written by
 * Daniel Engström
 */

#include <common.h>
#include <command.h>
#include <irq_func.h>
#include <asm/interrupt.h>

#if !CONFIG_IS_ENABLED(X86_64)

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	unsigned int count;
};

static struct irq_action irq_handlers[SYS_NUM_IRQS] = { {0} };
static int spurious_irq_cnt;
static int spurious_irq;

void irq_install_handler(int irq, interrupt_handler_t *handler, void *arg)
{
	int status;

	if (irq < 0 || irq >= SYS_NUM_IRQS) {
		printf("irq_install_handler: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler != NULL)
		printf("irq_install_handler: 0x%08lx replacing 0x%08lx\n",
				(ulong) handler,
				(ulong) irq_handlers[irq].handler);

	status = disable_interrupts();

	irq_handlers[irq].handler = handler;
	irq_handlers[irq].arg = arg;
	irq_handlers[irq].count = 0;

	if (CONFIG_IS_ENABLED(I8259_PIC))
		unmask_irq(irq);

	if (status)
		enable_interrupts();

	return;
}

void irq_free_handler(int irq)
{
	int status;

	if (irq < 0 || irq >= SYS_NUM_IRQS) {
		printf("irq_free_handler: bad irq number %d\n", irq);
		return;
	}

	status = disable_interrupts();

	if (CONFIG_IS_ENABLED(I8259_PIC))
		mask_irq(irq);

	irq_handlers[irq].handler = NULL;
	irq_handlers[irq].arg = NULL;

	if (status)
		enable_interrupts();

	return;
}

void do_irq(int hw_irq)
{
	int irq = hw_irq - 0x20;

	if (irq < 0 || irq >= SYS_NUM_IRQS) {
		printf("do_irq: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler) {
		if (CONFIG_IS_ENABLED(I8259_PIC))
			mask_irq(irq);

		irq_handlers[irq].handler(irq_handlers[irq].arg);
		irq_handlers[irq].count++;

		if (CONFIG_IS_ENABLED(I8259_PIC)) {
			unmask_irq(irq);
			specific_eoi(irq);
		}
	} else {
		if ((irq & 7) != 7) {
			spurious_irq_cnt++;
			spurious_irq = irq;
		}
	}
}
#endif

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
#if !CONFIG_IS_ENABLED(X86_64)
	struct idt_ptr ptr;
	int irq;

	interrupt_read_idt(&ptr);
	printf("IDT at %lx, size %x\n", ptr.address, ptr.size);
	printf("Spurious IRQ: %u, last unknown IRQ: %d\n",
			spurious_irq_cnt, spurious_irq);

	printf("Interrupt-Information:\n");
	printf("Nr  Routine   Arg       Count\n");

	for (irq = 0; irq < SYS_NUM_IRQS; irq++) {
		if (irq_handlers[irq].handler != NULL) {
			printf("%02d  %08lx  %08lx  %d\n",
					irq,
					(ulong)irq_handlers[irq].handler,
					(ulong)irq_handlers[irq].arg,
					irq_handlers[irq].count);
		}
	}
#endif

	return 0;
}
#endif

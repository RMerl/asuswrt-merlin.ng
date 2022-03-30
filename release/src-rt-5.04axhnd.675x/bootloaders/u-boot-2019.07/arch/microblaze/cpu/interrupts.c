// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/microblaze_intc.h>
#include <asm/asm.h>

DECLARE_GLOBAL_DATA_PTR;

void enable_interrupts(void)
{
	debug("Enable interrupts for the whole CPU\n");
	MSRSET(0x2);
}

int disable_interrupts(void)
{
	unsigned int msr;

	MFS(msr, rmsr);
	MSRCLR(0x2);
	return (msr & 0x2) != 0;
}

static struct irq_action *vecs;
static u32 irq_no;

/* mapping structure to interrupt controller */
microblaze_intc_t *intc;

/* default handler */
static void def_hdlr(void)
{
	puts("def_hdlr\n");
}

static void enable_one_interrupt(int irq)
{
	int mask;
	int offset = 1;

	offset <<= irq;
	mask = intc->ier;
	intc->ier = (mask | offset);

	debug("Enable one interrupt irq %x - mask %x,ier %x\n", offset, mask,
	      intc->ier);
	debug("INTC isr %x, ier %x, iar %x, mer %x\n", intc->isr, intc->ier,
	      intc->iar, intc->mer);
}

static void disable_one_interrupt(int irq)
{
	int mask;
	int offset = 1;

	offset <<= irq;
	mask = intc->ier;
	intc->ier = (mask & ~offset);

	debug("Disable one interrupt irq %x - mask %x,ier %x\n", irq, mask,
	      intc->ier);
	debug("INTC isr %x, ier %x, iar %x, mer %x\n", intc->isr, intc->ier,
	      intc->iar, intc->mer);
}

int install_interrupt_handler(int irq, interrupt_handler_t *hdlr, void *arg)
{
	struct irq_action *act;

	/* irq out of range */
	if ((irq < 0) || (irq > irq_no)) {
		puts("IRQ out of range\n");
		return -1;
	}
	act = &vecs[irq];
	if (hdlr) {		/* enable */
		act->handler = hdlr;
		act->arg = arg;
		act->count = 0;
		enable_one_interrupt(irq);
		return 0;
	}

	/* Disable */
	act->handler = (interrupt_handler_t *)def_hdlr;
	act->arg = (void *)irq;
	disable_one_interrupt(irq);
	return 1;
}

/* initialization interrupt controller - hardware */
static void intc_init(void)
{
	intc->mer = 0;
	intc->ier = 0;
	intc->iar = 0xFFFFFFFF;
	/* XIntc_Start - hw_interrupt enable and all interrupt enable */
	intc->mer = 0x3;

	debug("INTC isr %x, ier %x, iar %x, mer %x\n", intc->isr, intc->ier,
	      intc->iar, intc->mer);
}

int interrupt_init(void)
{
	int i;
	const void *blob = gd->fdt_blob;
	int node = 0;

	debug("INTC: Initialization\n");

	node = fdt_node_offset_by_compatible(blob, node,
				"xlnx,xps-intc-1.00.a");
	if (node != -1) {
		fdt_addr_t base = fdtdec_get_addr(blob, node, "reg");
		if (base == FDT_ADDR_T_NONE)
			return -1;

		debug("INTC: Base addr %lx\n", base);
		intc = (microblaze_intc_t *)base;
		irq_no = fdtdec_get_int(blob, node, "xlnx,num-intr-inputs", 0);
		debug("INTC: IRQ NO %x\n", irq_no);
	} else {
		return node;
	}

	if (irq_no) {
		vecs = calloc(1, sizeof(struct irq_action) * irq_no);
		if (vecs == NULL) {
			puts("Interrupt vector allocation failed\n");
			return -1;
		}

		/* initialize irq list */
		for (i = 0; i < irq_no; i++) {
			vecs[i].handler = (interrupt_handler_t *)def_hdlr;
			vecs[i].arg = (void *)i;
			vecs[i].count = 0;
		}
		/* initialize intc controller */
		intc_init();
		enable_interrupts();
	} else {
		puts("Undefined interrupt controller\n");
	}
	return 0;
}

void interrupt_handler(void)
{
	int irqs = intc->ivr;	/* find active interrupt */
	int mask = 1;
	int value;
	struct irq_action *act = vecs + irqs;

	debug("INTC isr %x, ier %x, iar %x, mer %x\n", intc->isr, intc->ier,
	      intc->iar, intc->mer);
#ifdef DEBUG
	R14(value);
#endif
	debug("Interrupt handler on %x line, r14 %x\n", irqs, value);

	debug("Jumping to interrupt handler rutine addr %x,count %x,arg %x\n",
	      (u32)act->handler, act->count, (u32)act->arg);
	act->handler(act->arg);
	act->count++;

	intc->iar = mask << irqs;

	debug("Dump INTC reg, isr %x, ier %x, iar %x, mer %x\n", intc->isr,
	      intc->ier, intc->iar, intc->mer);
#ifdef DEBUG
	R14(value);
#endif
	debug("Interrupt handler on %x line, r14 %x\n", irqs, value);
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, const char *argv[])
{
	int i;
	struct irq_action *act = vecs;

	if (irq_no) {
		puts("\nInterrupt-Information:\n\n"
		      "Nr  Routine   Arg       Count\n"
		      "-----------------------------\n");

		for (i = 0; i < irq_no; i++) {
			if (act->handler != (interrupt_handler_t *)def_hdlr) {
				printf("%02d  %08x  %08x  %d\n", i,
				       (int)act->handler, (int)act->arg,
				       act->count);
			}
			act++;
		}
		puts("\n");
	} else {
		puts("Undefined interrupt controller\n");
	}
	return 0;
}
#endif

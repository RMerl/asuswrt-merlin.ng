// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

/*
 * This file provides the interrupt handling functionality for systems
 * based on the standard PC/AT architecture using two cascaded i8259
 * Programmable Interrupt Controllers.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/i8259.h>
#include <asm/ibmpc.h>
#include <asm/interrupt.h>

int i8259_init(void)
{
	u8 i;

	/* Mask all interrupts */
	outb(0xff, MASTER_PIC + IMR);
	outb(0xff, SLAVE_PIC + IMR);

	/*
	 * Master PIC
	 * Place master PIC interrupts at INT20
	 */
	outb(ICW1_SEL | ICW1_EICW4, MASTER_PIC + ICW1);
	outb(0x20, MASTER_PIC + ICW2);
	outb(IR2, MASTER_PIC + ICW3);
	outb(ICW4_PM, MASTER_PIC + ICW4);

	for (i = 0; i < 8; i++)
		outb(OCW2_SEOI | i, MASTER_PIC + OCW2);

	/*
	 * Slave PIC
	 * Place slave PIC interrupts at INT28
	 */
	outb(ICW1_SEL | ICW1_EICW4, SLAVE_PIC + ICW1);
	outb(0x28, SLAVE_PIC + ICW2);
	outb(0x02, SLAVE_PIC + ICW3);
	outb(ICW4_PM, SLAVE_PIC + ICW4);

	for (i = 0; i < 8; i++)
		outb(OCW2_SEOI | i, SLAVE_PIC + OCW2);

	/*
	 * Enable cascaded interrupts by unmasking the cascade IRQ pin of
	 * the master PIC
	 */
	unmask_irq(2);

	/* Interrupt 9 should be level triggered (SCI). The OS might do this */
	configure_irq_trigger(9, true);

	return 0;
}

void mask_irq(int irq)
{
	int imr_port;

	if (irq >= SYS_NUM_IRQS)
		return;

	if (irq > 7)
		imr_port = SLAVE_PIC + IMR;
	else
		imr_port = MASTER_PIC + IMR;

	outb(inb(imr_port) | (1 << (irq & 7)), imr_port);
}

void unmask_irq(int irq)
{
	int imr_port;

	if (irq >= SYS_NUM_IRQS)
		return;

	if (irq > 7)
		imr_port = SLAVE_PIC + IMR;
	else
		imr_port = MASTER_PIC + IMR;

	outb(inb(imr_port) & ~(1 << (irq & 7)), imr_port);
}

void specific_eoi(int irq)
{
	if (irq >= SYS_NUM_IRQS)
		return;

	if (irq > 7) {
		/*
		 *  IRQ is on the slave - Issue a corresponding EOI to the
		 *  slave PIC and an EOI for IRQ2 (the cascade interrupt)
		 *  on the master PIC
		 */
		outb(OCW2_SEOI | (irq & 7), SLAVE_PIC + OCW2);
		irq = SEOI_IR2;
	}

	outb(OCW2_SEOI | irq, MASTER_PIC + OCW2);
}

void configure_irq_trigger(int int_num, bool is_level_triggered)
{
	u16 int_bits = inb(ELCR1) | (((u16)inb(ELCR2)) << 8);

	debug("%s: current interrupts are 0x%x\n", __func__, int_bits);
	if (is_level_triggered)
		int_bits |= (1 << int_num);
	else
		int_bits &= ~(1 << int_num);

	/* Write new values */
	debug("%s: try to set interrupts 0x%x\n", __func__, int_bits);
	outb((u8)(int_bits & 0xff), ELCR1);
	outb((u8)(int_bits >> 8), ELCR2);
}

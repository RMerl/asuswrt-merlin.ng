/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.cz>
 */

typedef volatile struct microblaze_intc_t {
	int isr; /* interrupt status register */
	int ipr; /* interrupt pending register */
	int ier; /* interrupt enable register */
	int iar; /* interrupt acknowledge register */
	int sie; /* set interrupt enable bits */
	int cie; /* clear interrupt enable bits */
	int ivr; /* interrupt vector register */
	int mer; /* master enable register */
} microblaze_intc_t;

struct irq_action {
	interrupt_handler_t *handler; /* pointer to interrupt rutine */
	void *arg;
	int count; /* number of interrupt */
};

/**
 * Register and unregister interrupt handler rutines
 *
 * @param irq	IRQ number
 * @param hdlr	Interrupt handler rutine
 * @param arg	Pointer to argument which is passed to int. handler rutine
 * @return	0 if registration pass, 1 if unregistration pass,
 *		or an error code < 0 otherwise
 */
int install_interrupt_handler(int irq, interrupt_handler_t *hdlr,
				       void *arg);

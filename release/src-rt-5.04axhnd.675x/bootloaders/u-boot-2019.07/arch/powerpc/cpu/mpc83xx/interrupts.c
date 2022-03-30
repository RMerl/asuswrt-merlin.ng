// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2004 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	ulong count;
};

void interrupt_init_cpu (unsigned *decrementer_count)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	*decrementer_count = (gd->bus_clk / 4) / CONFIG_SYS_HZ;

	/* Enable e300 time base */

	immr->sysconf.spcr |= 0x00400000;
}


/*
 * Handle external interrupts
 */

void external_interrupt (struct pt_regs *regs)
{
}


/*
 * Install and free an interrupt handler.
 */

void
irq_install_handler (int irq, interrupt_handler_t * handler, void *arg)
{
}


void irq_free_handler (int irq)
{
}


void timer_interrupt_cpu (struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}


#if defined(CONFIG_CMD_IRQ)

/* ripped this out of ppc4xx/interrupts.c */

/*
 * irqinfo - print information about PCI devices
 */

void
do_irqinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char * const argv[])
{
}

#endif

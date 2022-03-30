// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <command.h>
#include <asm/nios2.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/ptrace.h>

/*************************************************************************/
struct	irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action vecs[32];

int disable_interrupts (void)
{
	int val = rdctl (CTL_STATUS);
	wrctl (CTL_STATUS, val & ~STATUS_IE);
	return (val & STATUS_IE);
}

void enable_interrupts( void )
{
	int val = rdctl (CTL_STATUS);
	wrctl (CTL_STATUS, val | STATUS_IE);
}

void external_interrupt (struct pt_regs *regs)
{
	unsigned irqs;
	struct irq_action *act;

	/* Evaluate only irqs that are both enabled AND pending */
	irqs = rdctl (CTL_IENABLE) & rdctl (CTL_IPENDING);
	act = vecs;

	/* Assume (as does the Nios2 HAL) that bit 0 is highest
	 * priority. NOTE: There is ALWAYS a handler assigned
	 * (the default if no other).
	 */
	while (irqs) {
		if (irqs & 1) {
			act->handler (act->arg);
			act->count++;
		}
		irqs >>=1;
		act++;
	}
}

static void def_hdlr (void *arg)
{
	unsigned irqs = rdctl (CTL_IENABLE);

	/* Disable the individual interrupt -- with gratuitous
	 * warning.
	 */
	irqs &= ~(1 << (int)arg);
	wrctl (CTL_IENABLE, irqs);
	printf ("WARNING: Disabling unhandled interrupt: %d\n",
			(int)arg);
}

/*************************************************************************/
void irq_install_handler (int irq, interrupt_handler_t *hdlr, void *arg)
{

	int flag;
	struct irq_action *act;
	unsigned ena = rdctl (CTL_IENABLE);

	if ((irq < 0) || (irq > 31))
		return;
	act = &vecs[irq];

	flag = disable_interrupts ();
	if (hdlr) {
		act->handler = hdlr;
		act->arg = arg;
		ena |= (1 << irq);		/* enable */
	} else {
		act->handler = def_hdlr;
		act->arg = (void *)irq;
		ena &= ~(1 << irq);		/* disable */
	}
	wrctl (CTL_IENABLE, ena);
	if (flag) enable_interrupts ();
}


int interrupt_init (void)
{
	int i;

	/* Assign the default handler to all */
	for (i = 0; i < 32; i++) {
		vecs[i].handler = def_hdlr;
		vecs[i].arg = (void *)i;
		vecs[i].count = 0;
	}

	enable_interrupts ();
	return (0);
}


/*************************************************************************/
#if defined(CONFIG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	struct irq_action *act = vecs;

	printf ("\nInterrupt-Information:\n\n");
	printf ("Nr  Routine   Arg       Count\n");
	printf ("-----------------------------\n");

	for (i=0; i<32; i++) {
		if (act->handler != def_hdlr) {
			printf ("%02d  %08lx  %08lx  %d\n",
				i,
				(ulong)act->handler,
				(ulong)act->arg,
				act->count);
		}
		act++;
	}
	printf ("\n");

	return (0);
}
#endif

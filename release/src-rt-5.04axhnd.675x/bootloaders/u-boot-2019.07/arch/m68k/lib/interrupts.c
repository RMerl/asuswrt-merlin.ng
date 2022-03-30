// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor Inc
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <watchdog.h>
#include <asm/processor.h>
#include <asm/immap.h>

#define	NR_IRQS		(CONFIG_SYS_NUM_IRQS)

/*
 * Interrupt vector functions.
 */
struct interrupt_action {
	interrupt_handler_t *handler;
	void *arg;
};

static struct interrupt_action irq_vecs[NR_IRQS];

static __inline__ unsigned short get_sr (void)
{
	unsigned short sr;

	asm volatile ("move.w %%sr,%0":"=r" (sr):);

	return sr;
}

static __inline__ void set_sr (unsigned short sr)
{
	asm volatile ("move.w %0,%%sr"::"r" (sr));
}

/************************************************************************/
/*
 * Install and free an interrupt handler
 */
void irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{
	if ((vec < 0) || (vec >= NR_IRQS)) {
		printf ("irq_install_handler: wrong interrupt vector %d\n",
			vec);
		return;
	}

	irq_vecs[vec].handler = handler;
	irq_vecs[vec].arg = arg;
}

void irq_free_handler (int vec)
{
	if ((vec < 0) || (vec >= NR_IRQS)) {
		return;
	}

	irq_vecs[vec].handler = NULL;
	irq_vecs[vec].arg = NULL;
}

void enable_interrupts (void)
{
	unsigned short sr;

	sr = get_sr ();
	set_sr (sr & ~0x0700);
}

int disable_interrupts (void)
{
	unsigned short sr;

	sr = get_sr ();
	set_sr (sr | 0x0700);

	return ((sr & 0x0700) == 0);	/* return true, if interrupts were enabled before */
}

void int_handler (struct pt_regs *fp)
{
	int vec;

	vec = (fp->vector >> 2) & 0xff;
	if (vec > 0x40)
		vec -= 0x40;

	if (irq_vecs[vec].handler != NULL) {
		irq_vecs[vec].handler (irq_vecs[vec].arg);
	} else {
		printf ("\nBogus External Interrupt Vector %d\n", vec);
	}
}

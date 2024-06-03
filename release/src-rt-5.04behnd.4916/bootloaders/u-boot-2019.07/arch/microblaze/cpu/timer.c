// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/microblaze_timer.h>
#include <asm/microblaze_intc.h>

DECLARE_GLOBAL_DATA_PTR;

volatile int timestamp = 0;
microblaze_timer_t *tmr;

ulong get_timer (ulong base)
{
	if (tmr)
		return timestamp - base;
	return timestamp++ - base;
}

void __udelay(unsigned long usec)
{
	u32 i;

	if (tmr) {
		i = get_timer(0);
		while ((get_timer(0) - i) < (usec / 1000))
			;
	}
}

#ifndef CONFIG_SPL_BUILD
static void timer_isr(void *arg)
{
	timestamp++;
	tmr->control = tmr->control | TIMER_INTERRUPT;
}

int timer_init (void)
{
	int irq = -1;
	u32 preload = 0;
	u32 ret = 0;
	const void *blob = gd->fdt_blob;
	int node = 0;
	u32 cell[2];

	debug("TIMER: Initialization\n");

	/* Do not init before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	node = fdt_node_offset_by_compatible(blob, node,
				"xlnx,xps-timer-1.00.a");
	if (node != -1) {
		fdt_addr_t base = fdtdec_get_addr(blob, node, "reg");
		if (base == FDT_ADDR_T_NONE)
			return -1;

		debug("TIMER: Base addr %lx\n", base);
		tmr = (microblaze_timer_t *)base;

		ret = fdtdec_get_int_array(blob, node, "interrupts",
					    cell, ARRAY_SIZE(cell));
		if (ret)
			return ret;

		irq = cell[0];
		debug("TIMER: IRQ %x\n", irq);

		preload = fdtdec_get_int(blob, node, "clock-frequency", 0);
		preload /= CONFIG_SYS_HZ;
	} else {
		return node;
	}

	if (tmr && preload && irq >= 0) {
		tmr->loadreg = preload;
		tmr->control = TIMER_INTERRUPT | TIMER_RESET;
		tmr->control = TIMER_ENABLE | TIMER_ENABLE_INTR |\
					TIMER_RELOAD | TIMER_DOWN_COUNT;
		timestamp = 0;
		ret = install_interrupt_handler (irq, timer_isr, (void *)tmr);
		if (ret)
			tmr = NULL;
	}
	/* No problem if timer is not found/initialized */
	return 0;
}
#else
int timer_init(void)
{
	return 0;
}
#endif

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On Microblaze it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On Microblaze it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

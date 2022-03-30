// SPDX-License-Identifier: GPL-2.0+

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_reserve_stacks(void)
{
	/* reserve space for exception vector table */
	gd->start_addr_sp -= 0x500;
	gd->start_addr_sp &= ~0xFFF;
	gd->irq_sp = gd->start_addr_sp;
	debug("Reserving %d Bytes for exception vector at: %08lx\n",
	      0x500, gd->start_addr_sp);

	return 0;
}

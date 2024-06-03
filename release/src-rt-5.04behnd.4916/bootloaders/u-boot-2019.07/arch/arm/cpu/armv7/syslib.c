// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 */

#include <common.h>
#include <asm/io.h>

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
 *************************************************************/
void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, void *read_addr,
		  u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = readl((u32)read_addr) & read_bit_mask;
		if (val == match_value)
			return 1;
		if (i == bound)
			return 0;
	} while (1);
}

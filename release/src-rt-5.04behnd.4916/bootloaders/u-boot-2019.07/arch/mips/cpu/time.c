// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/mipsregs.h>

unsigned long notrace timer_read_counter(void)
{
	return read_c0_count();
}

ulong notrace get_tbclk(void)
{
	return CONFIG_SYS_MIPS_TIMER_FREQ;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 */

#include <irq_func.h>

int interrupt_init(void)
{
	return 0;
}

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}

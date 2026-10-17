// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <command.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void trap_init(unsigned long reloc_addr);

int arch_initr_trap(void)
{
	trap_init(gd->relocaddr);

	return 0;
}

#ifndef CONFIG_SYSRESET
void reset_cpu(void)
{
	/* TODO: Refactor all the do_reset calls to be reset_cpu() instead */
	do_reset(NULL, 0, 0, NULL);
}
#endif

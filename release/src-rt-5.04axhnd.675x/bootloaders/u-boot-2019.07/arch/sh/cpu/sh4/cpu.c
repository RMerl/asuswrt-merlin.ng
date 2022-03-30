// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/processor.h>

int checkcpu(void)
{
	puts("CPU: SH4\n");
	return 0;
}

int cpu_init (void)
{
	return 0;
}

int cleanup_before_linux (void)
{
	disable_interrupts();
	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	reset_cpu (0);
	return 0;
}

int cpu_eth_init(bd_t *bis)
{
#ifdef CONFIG_SH_ETHER
	sh_eth_initialize(bis);
#endif
	return 0;
}

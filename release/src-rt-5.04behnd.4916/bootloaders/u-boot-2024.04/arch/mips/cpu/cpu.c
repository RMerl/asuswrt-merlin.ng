// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 */

#include <cpu_func.h>
#include <command.h>
#include <init.h>
#include <linux/compiler.h>
#include <asm/cache.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>

#if !CONFIG_IS_ENABLED(SYSRESET)
void __weak _machine_restart(void)
{
	puts("*** reset failed ***\n");

	while (1)
		/* NOP */;
}

void reset_cpu(void)
{
	_machine_restart();
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	reset_cpu();

	return 0;
}
#endif

int arch_cpu_init(void)
{
	mips_cache_probe();
	return 0;
}

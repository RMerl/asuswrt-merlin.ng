// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <common.h>
#include <hang.h>
#include <asm/ptrace.h>

void trap_handler (struct pt_regs *regs)
{
	/* Just issue warning */
	printf ("\n\n*** WARNING: unimplemented trap @ %08x\n\n",
			regs->reg[29] - 4);
}

void soft_emulation (struct pt_regs *regs)
{
	/* TODO: Software emulation of mul/div etc. Until this is
	 * implemented, generate warning and hang.
	 */
	printf ("\n\n*** ERROR: unimplemented instruction @ %08x\n",
			regs->reg[29] - 4);
	hang();
}

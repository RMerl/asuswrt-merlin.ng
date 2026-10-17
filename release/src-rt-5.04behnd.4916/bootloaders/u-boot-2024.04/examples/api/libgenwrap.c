// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 *
 * This is is a set of wrappers/stubs that allow to use certain routines from
 * U-Boot's lib in the standalone app. This way way we can re-use
 * existing code e.g. operations on strings and similar.
 */

#include <common.h>
#include <command.h>
#include <hang.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <api_public.h>

#include "glue.h"

void putc(const char c)
{
	ub_putc(c);
}

void puts(const char *s)
{
	ub_puts(s);
}

void __udelay(unsigned long usec)
{
	ub_udelay(usec);
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ub_reset();
	return 0;
}

void *malloc (size_t len)
{
	return NULL;
}

void hang(void)
{
	while (1) ;
}

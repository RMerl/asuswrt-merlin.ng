// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/u-boot-x86.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long do_go_exec(ulong (*entry)(int, char * const []),
			 int argc, char * const argv[])
{
	unsigned long ret = 0;
	char **argv_tmp;

	/*
	 * x86 does not use a dedicated register to pass the pointer to
	 * the global_data, so it is instead passed as argv[-1]. By using
	 * argv[-1], the called 'Application' can use the contents of
	 * argv natively. However, to safely use argv[-1] a new copy of
	 * argv is needed with the extra element
	 */
	argv_tmp = malloc(sizeof(char *) * (argc + 1));

	if (argv_tmp) {
		argv_tmp[0] = (char *)gd;

		memcpy(&argv_tmp[1], argv, (size_t)(sizeof(char *) * argc));

		ret = (entry) (argc, &argv_tmp[1]);
		free(argv_tmp);
	}

	return ret;
}

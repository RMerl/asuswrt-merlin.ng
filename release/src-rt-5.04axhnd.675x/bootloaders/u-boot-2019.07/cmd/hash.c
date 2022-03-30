// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <hash.h>
#include <linux/ctype.h>

static int do_hash(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s;
	int flags = HASH_FLAG_ENV;

#ifdef CONFIG_HASH_VERIFY
	if (argc < 4)
		return CMD_RET_USAGE;
	if (!strcmp(argv[1], "-v")) {
		flags |= HASH_FLAG_VERIFY;
		argc--;
		argv++;
	}
#endif
	/* Move forward to 'algorithm' parameter */
	argc--;
	argv++;
	for (s = *argv; *s; s++)
		*s = tolower(*s);
	return hash_command(*argv, flags, cmdtp, flag, argc - 1, argv + 1);
}

#ifdef CONFIG_HASH_VERIFY
#define HARGS 6
#else
#define HARGS 5
#endif

U_BOOT_CMD(
	hash,	HARGS,	1,	do_hash,
	"compute hash message digest",
	"algorithm address count [[*]hash_dest]\n"
		"    - compute message digest [save to env var / *address]"
#ifdef CONFIG_HASH_VERIFY
	"\nhash -v algorithm address count [*]hash\n"
		"    - verify message digest of memory area to immediate value, \n"
		"      env var or *address"
#endif
);

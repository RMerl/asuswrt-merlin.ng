// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <hash.h>
#include <u-boot/sha1.h>

int do_sha1sum(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int flags = HASH_FLAG_ENV;
	int ac;
	char * const *av;

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;
	ac = argc - 1;
#ifdef CONFIG_SHA1SUM_VERIFY
	if (strcmp(*av, "-v") == 0) {
		flags |= HASH_FLAG_VERIFY;
		av++;
		ac--;
	}
#endif

	return hash_command("sha1", flags, cmdtp, flag, ac, av);
}

#ifdef CONFIG_SHA1SUM_VERIFY
U_BOOT_CMD(
	sha1sum,	5,	1,	do_sha1sum,
	"compute SHA1 message digest",
	"address count [[*]sum]\n"
		"    - compute SHA1 message digest [save to sum]\n"
	"sha1sum -v address count [*]sum\n"
		"    - verify sha1sum of memory area"
);
#else
U_BOOT_CMD(
	sha1sum,	4,	1,	do_sha1sum,
	"compute SHA1 message digest",
	"address count [[*]sum]\n"
		"    - compute SHA1 message digest [save to sum]"
);
#endif

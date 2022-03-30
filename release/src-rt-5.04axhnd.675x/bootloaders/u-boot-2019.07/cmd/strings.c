/*
 * cmd_strings.c - just like `strings` command
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <common.h>
#include <command.h>

static char *start_addr, *last_addr;

int do_strings(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 1)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		start_addr = (char *)simple_strtoul(argv[1], NULL, 16);
		if (argc > 2)
			last_addr = (char *)simple_strtoul(argv[2], NULL, 16);
		else
			last_addr = (char *)-1;
	}

	char *addr = start_addr;
	do {
		puts(addr);
		puts("\n");
		addr += strlen(addr) + 1;
	} while (addr[0] && addr < last_addr);

	last_addr = addr + (last_addr - start_addr);
	start_addr = addr;

	return 0;
}

U_BOOT_CMD(
	strings, 3, 1, do_strings,
	"display strings",
	"<addr> [byte count]\n"
	"    - display strings at <addr> for at least [byte count] or first double NUL"
);

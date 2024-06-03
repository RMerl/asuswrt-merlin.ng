// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

/*
 * IO space access commands.
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

/*
 * IO Display
 *
 * Syntax:
 *	iod{.b, .w, .l} {addr}
 */
int do_io_iod(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr;
	int size;

	if (argc != 2)
		return CMD_RET_USAGE;

	size = cmd_get_data_size(argv[0], 4);
	if (size < 0)
		return 1;

	addr = simple_strtoul(argv[1], NULL, 16);

	printf("%04x: ", (u16) addr);

	if (size == 4)
		printf("%08x\n", inl(addr));
	else if (size == 2)
		printf("%04x\n", inw(addr));
	else
		printf("%02x\n", inb(addr));

	return 0;
}

int do_io_iow(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr, val;
	int size;

	if (argc != 3)
		return CMD_RET_USAGE;

	size = cmd_get_data_size(argv[0], 4);
	if (size < 0)
		return 1;

	addr = simple_strtoul(argv[1], NULL, 16);
	val = simple_strtoul(argv[2], NULL, 16);

	if (size == 4)
		outl((u32) val, addr);
	else if (size == 2)
		outw((u16) val, addr);
	else
		outb((u8) val, addr);

	return 0;
}

/**************************************************/
U_BOOT_CMD(iod, 2, 0, do_io_iod,
	   "IO space display", "[.b, .w, .l] address");

U_BOOT_CMD(iow, 3, 0, do_io_iow,
	   "IO space modify",
	   "[.b, .w, .l] address value");

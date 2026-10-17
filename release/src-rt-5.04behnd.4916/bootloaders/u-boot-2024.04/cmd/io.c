// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

/*
 * IO space access commands.
 */

#include <common.h>
#include <command.h>
#include <display_options.h>
#include <asm/io.h>

/* Display values from last command */
static ulong last_addr, last_size;
static ulong last_length = 0x40;
static ulong base_address;

#define DISP_LINE_LEN	16

/*
 * IO Display
 *
 * Syntax:
 *	iod{.b, .w, .l} {addr}
 */
int do_io_iod(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr, length, bytes;
	u8 buf[DISP_LINE_LEN];
	int size, todo;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	addr = last_addr;
	size = last_size;
	length = last_length;

	if (argc < 2)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		size = cmd_get_data_size(argv[0], 4);
		if (size < 0)
			return 1;

		/* Address is specified since argc > 1 */
		addr = hextoul(argv[1], NULL);
		addr += base_address;

		/*
		 * If another parameter, it is the length to display.
		 * Length is the number of objects, not number of bytes.
		 */
		if (argc > 2)
			length = hextoul(argv[2], NULL);
	}

	bytes = size * length;

	/* Print the lines */
	for (; bytes > 0; addr += todo) {
		u8 *ptr = buf;
		int i;

		todo = min(bytes, (ulong)DISP_LINE_LEN);
		for (i = 0; i < todo; i += size, ptr += size) {
			if (size == 4)
				*(u32 *)ptr = inl(addr + i);
			else if (size == 2)
				*(u16 *)ptr = inw(addr + i);
			else
				*ptr = inb(addr + i);
		}
		print_buffer(addr, buf, size, todo / size,
			     DISP_LINE_LEN / size);
		bytes -= todo;
	}

	last_addr = addr;
	last_length = length;
	last_size = size;

	return 0;
}

int do_io_iow(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong addr, val;
	int size;

	if (argc != 3)
		return CMD_RET_USAGE;

	size = cmd_get_data_size(argv[0], 4);
	if (size < 0)
		return 1;

	addr = hextoul(argv[1], NULL);
	val = hextoul(argv[2], NULL);

	if (size == 4)
		outl((u32) val, addr);
	else if (size == 2)
		outw((u16) val, addr);
	else
		outb((u8) val, addr);

	return 0;
}

/**************************************************/
U_BOOT_CMD(iod, 3, 1, do_io_iod,
	   "IO space display", "[.b, .w, .l] address");

U_BOOT_CMD(iow, 3, 0, do_io_iow,
	   "IO space modify",
	   "[.b, .w, .l] address value");

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * based on cmd_mem.c
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <console.h>

#include <gdsys_fpga.h>

static uint	dp_last_fpga;
static uint	dp_last_addr;
static uint	dp_last_length = 0x40;

/*
 * FPGA Memory Display
 *
 * Syntax:
 *	fpgad {fpga} {addr} {len}
 */
#define DISP_LINE_LEN	16
int do_fpga_md(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int k;
	unsigned int fpga;
	ulong	addr, length;
	int rc = 0;
	u16 linebuf[DISP_LINE_LEN/sizeof(u16)];
	ulong nbytes;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	fpga = dp_last_fpga;
	addr = dp_last_addr;
	length = dp_last_length;

	if (argc < 3)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * FPGA is specified since argc > 2
		 */
		fpga = simple_strtoul(argv[1], NULL, 16);

		/*
		 * Address is specified since argc > 2
		 */
		addr = simple_strtoul(argv[2], NULL, 16);

		/*
		 * If another parameter, it is the length to display.
		 * Length is the number of objects, not number of bytes.
		 */
		if (argc > 3)
			length = simple_strtoul(argv[3], NULL, 16);
	}

	nbytes = length * sizeof(u16);
	do {
		ulong linebytes = (nbytes > DISP_LINE_LEN) ?
				  DISP_LINE_LEN : nbytes;

		for (k = 0; k < linebytes / sizeof(u16); ++k)
			fpga_get_reg(fpga,
				     (u16 *)fpga_ptr[fpga] + addr
				     / sizeof(u16) + k,
				     addr + k * sizeof(u16),
				     &linebuf[k]);
		print_buffer(addr, (void *)linebuf, sizeof(u16),
			     linebytes / sizeof(u16),
			     DISP_LINE_LEN / sizeof(u16));

		nbytes -= linebytes;
		addr += linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);

	dp_last_fpga = fpga;
	dp_last_addr = addr;
	dp_last_length = length;
	return rc;
}

U_BOOT_CMD(
	fpgad,	4,	1,	do_fpga_md,
	"fpga register display",
	"fpga address [# of objects]"
);

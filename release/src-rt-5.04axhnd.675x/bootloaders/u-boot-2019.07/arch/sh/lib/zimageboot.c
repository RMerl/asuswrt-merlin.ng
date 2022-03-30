// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 *   Renesas Solutions Corp.
 *   Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

/*
 * Linux SuperH zImage loading and boot
 */

#include <common.h>
#include <asm/io.h>
#include <asm/zimage.h>

int do_sh_zimageboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong (*zboot_entry)(int, char * const []) = NULL;
	char *s0, *s1;
	unsigned char *param = NULL;
	char *cmdline;
	char *bootargs;

	disable_interrupts();

	if (argc >= 3) {
		/* argv[1] holds the address of the zImage */
		s0 = argv[1];
		/* argv[2] holds the address of zero page */
		s1 = argv[2];
	} else {
		goto exit;
	}

	if (s0)
		zboot_entry = (ulong (*)(int, char * const []))simple_strtoul(s0, NULL, 16);

	/* empty_zero_page */
	if (s1)
		param = (unsigned char*)simple_strtoul(s1, NULL, 16);

	/* Linux kernel command line */
	cmdline = (char *)param + COMMAND_LINE;
	bootargs = env_get("bootargs");

	/* Clear zero page */
	/* cppcheck-suppress nullPointer */
	memset(param, 0, 0x1000);

	/* Set commandline */
	strcpy(cmdline, bootargs);

	/* Boot */
	zboot_entry(0, NULL);

exit:
	return -1;
}

U_BOOT_CMD(
	zimageboot, 3, 0,	do_sh_zimageboot,
	"Boot zImage for Renesas SH",
	""
);

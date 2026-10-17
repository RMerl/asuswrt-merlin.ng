// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>
#include <stdlib.h>

static int do_bmp_info(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	ulong addr;

	switch (argc) {
	case 1:		/* use image_load_addr as default address */
		addr = image_load_addr;
		break;
	case 2:		/* use argument */
		addr = hextoul(argv[1], NULL);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return (bmp_info(addr));
}

static int do_bmp_display(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	ulong addr;
	int x = 0, y = 0;

	splash_get_pos(&x, &y);

	switch (argc) {
	case 1:		/* use image_load_addr as default address */
		addr = image_load_addr;
		break;
	case 2:		/* use argument */
		addr = hextoul(argv[1], NULL);
		break;
	case 4:
		addr = hextoul(argv[1], NULL);
		if (!strcmp(argv[2], "m"))
			x = BMP_ALIGN_CENTER;
		else
			x = dectoul(argv[2], NULL);
		if (!strcmp(argv[3], "m"))
			y = BMP_ALIGN_CENTER;
		else
			y = dectoul(argv[3], NULL);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return (bmp_display(addr, x, y));
}

static struct cmd_tbl cmd_bmp_sub[] = {
	U_BOOT_CMD_MKENT(info, 3, 0, do_bmp_info, "", ""),
	U_BOOT_CMD_MKENT(display, 5, 0, do_bmp_display, "", ""),
};

static int do_bmp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *c;

	/* Strip off leading 'bmp' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_bmp_sub[0], ARRAY_SIZE(cmd_bmp_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	bmp,	5,	1,	do_bmp,
	"manipulate BMP image data",
	"info <imageAddr>          - display image info\n"
	"bmp display <imageAddr> [x y] - display image at x,y"
);

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on the gdsys osd driver, which is
 *
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <hexdump.h>
#include <video_osd.h>
#include <malloc.h>

static int do_osd_write(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *dev;
	uint x, y;
	uint count;
	char *hexstr;
	u8 *buffer;
	size_t buflen;
	int res;

	if (argc < 4 || (strlen(argv[3])) % 2)
		return CMD_RET_USAGE;

	x = hextoul(argv[1], NULL);
	y = hextoul(argv[2], NULL);
	hexstr = argv[3];
	count = (argc > 4) ? hextoul(argv[4], NULL) : 1;

	buflen = strlen(hexstr) / 2;

	buffer = malloc(buflen);
	if (!buffer) {
		puts("Memory allocation failure\n");
		return CMD_RET_FAILURE;
	}

	res = hex2bin(buffer, hexstr, buflen);
	if (res) {
		free(buffer);
		puts("Hexadecimal input contained invalid characters\n");
		return CMD_RET_FAILURE;
	}

	for (uclass_first_device(UCLASS_VIDEO_OSD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		int res;

		res = video_osd_set_mem(dev, x, y, buffer, buflen, count);
		if (res) {
			free(buffer);
			printf("Could not write to video mem on osd %s\n",
			       dev->name);
			return CMD_RET_FAILURE;
		}
	}

	free(buffer);

	return CMD_RET_SUCCESS;
}

static int do_osd_print(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *dev;
	uint x, y;
	u8 color;
	char *text;

	if (argc < 5)
		return CMD_RET_USAGE;

	x = hextoul(argv[1], NULL);
	y = hextoul(argv[2], NULL);
	color = hextoul(argv[3], NULL);
	text = argv[4];

	for (uclass_first_device(UCLASS_VIDEO_OSD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		int res;

		res = video_osd_print(dev, x, y, color, text);
		if (res) {
			printf("Could not print string to osd %s\n", dev->name);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

static int do_osd_size(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	uint x, y;

	if (argc < 3)
		return CMD_RET_USAGE;

	x = hextoul(argv[1], NULL);
	y = hextoul(argv[2], NULL);

	for (uclass_first_device(UCLASS_VIDEO_OSD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		int res;

		res = video_osd_set_size(dev, x, y);

		if (res) {
			printf("Could not set size on osd %s\n", dev->name);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	osdw, 5, 0, do_osd_write,
	"write 16-bit hex encoded buffer to osd memory",
	"osdw [pos_x] [pos_y] [buffer] [count] - write 8-bit hex encoded buffer to osd memory\n"
);

U_BOOT_CMD(
	osdp, 5, 0, do_osd_print,
	"write ASCII buffer to osd memory",
	"osdp [pos_x] [pos_y] [color] [text] - write ASCII buffer to osd memory\n"
);

U_BOOT_CMD(
	osdsize, 3, 0, do_osd_size,
	"set OSD XY size in characters",
	"osdsize [size_x] [size_y] - set OSD XY size in characters\n"
);

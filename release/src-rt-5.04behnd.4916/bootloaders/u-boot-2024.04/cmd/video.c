// SPDX-License-Identifier: GPL-2.0+
/*
 * video commands
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <video.h>
#include <video_console.h>

static int do_video_setcursor(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	unsigned int col, row;
	struct udevice *dev;

	if (argc != 3)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	col = dectoul(argv[1], NULL);
	row = dectoul(argv[2], NULL);
	vidconsole_position_cursor(dev, col, row);

	return 0;
}

static int do_video_puts(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct udevice *dev;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	ret = vidconsole_put_string(dev, argv[1]);
	if (!ret)
		ret = video_sync(dev->parent, false);

	return ret ? CMD_RET_FAILURE : 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_video_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_video_puts,
	"print string on video framebuffer",
	"    <string>"
);

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

static int do_font_list(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *dev;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	vidconsole_list_fonts(dev);

	return 0;
}

static int do_font_select(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct udevice *dev;
	const char *name;
	uint size = 0;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	name = argv[1];
	if (argc == 3)
		size = dectoul(argv[2], NULL);
	ret = vidconsole_select_font(dev, name, size);
	if (ret) {
		printf("Failed (error %d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}
static int do_font_size(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	const char *font_name;
	struct udevice *dev;
	uint size;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev))
		return CMD_RET_FAILURE;
	ret = vidconsole_get_font_size(dev, &font_name, &size);
	if (ret) {
		printf("Failed (error %d)\n", ret);
		return CMD_RET_FAILURE;
	}

	size = dectoul(argv[1], NULL);

	ret = vidconsole_select_font(dev, font_name, size);
	if (ret) {
		printf("Failed (error %d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}


U_BOOT_LONGHELP(font,
	"list       - list available fonts\n"
	"font select <name> [<size>] - select font to use\n"
	"font size <size> - select font size to");

U_BOOT_CMD_WITH_SUBCMDS(font, "Fonts", font_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_font_list),
	U_BOOT_SUBCMD_MKENT(select, 3, 1, do_font_select),
	U_BOOT_SUBCMD_MKENT(size, 2, 1, do_font_size));

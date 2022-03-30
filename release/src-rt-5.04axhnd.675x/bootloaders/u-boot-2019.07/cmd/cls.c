// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * DENX Software Engineering, Anatolij Gustschin <agust@denx.de>
 *
 * cls - clear screen command
 */
#include <common.h>
#include <command.h>
#include <dm.h>
#include <lcd.h>
#include <video.h>

static int do_video_clear(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
#if defined(CONFIG_DM_VIDEO)
	struct udevice *dev;

	if (uclass_first_device_err(UCLASS_VIDEO, &dev))
		return CMD_RET_FAILURE;

	if (video_clear(dev))
		return CMD_RET_FAILURE;
#elif defined(CONFIG_CFB_CONSOLE)
	video_clear();
#elif defined(CONFIG_LCD)
	lcd_clear();
#else
	return CMD_RET_FAILURE;
#endif
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(cls,	1, 1, do_video_clear, "clear screen", "");

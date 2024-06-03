// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <fdtdec.h>
#include <sound.h>

DECLARE_GLOBAL_DATA_PTR;

/* Initilaise sound subsystem */
static int do_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_SOUND, &dev);
	if (!ret)
		ret = sound_setup(dev);
	if (ret) {
		printf("Initialise Audio driver failed (ret=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

/* play sound from buffer */
static int do_play(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	int ret = 0;
	int msec = 1000;
	int freq = 400;

	if (argc > 1)
		msec = simple_strtoul(argv[1], NULL, 10);
	if (argc > 2)
		freq = simple_strtoul(argv[2], NULL, 10);

	ret = uclass_first_device_err(UCLASS_SOUND, &dev);
	if (!ret)
		ret = sound_beep(dev, msec, freq);
	if (ret) {
		printf("Sound device failed to play (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static cmd_tbl_t cmd_sound_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_init, "", ""),
	U_BOOT_CMD_MKENT(play, 2, 1, do_play, "", ""),
};

/* process sound command */
static int do_sound(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 1)
		return CMD_RET_USAGE;

	/* Strip off leading 'sound' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sound_sub[0], ARRAY_SIZE(cmd_sound_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sound, 4, 1, do_sound,
	"sound sub-system",
	"init - initialise the sound driver\n"
	"sound play [len] [freq] - play a sound for len ms at freq hz\n"
);

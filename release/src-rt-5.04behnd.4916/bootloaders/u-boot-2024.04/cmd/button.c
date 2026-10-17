// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Based on led.c
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <button.h>
#include <dm/uclass-internal.h>

static const char *const state_label[] = {
	[BUTTON_OFF]	= "off",
	[BUTTON_ON]	= "on",
};

static int show_button_state(struct udevice *dev)
{
	int ret;

	ret = button_get_state(dev);
	if (ret >= BUTTON_COUNT)
		ret = -EINVAL;
	if (ret >= 0)
		printf("%s\n", state_label[ret]);

	return ret;
}

static int list_buttons(void)
{
	struct udevice *dev;
	int ret;

	for (uclass_find_first_device(UCLASS_BUTTON, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct button_uc_plat *plat = dev_get_uclass_plat(dev);

		if (!plat->label)
			continue;
		printf("%-15s ", plat->label);
		if (device_active(dev)) {
			ret = show_button_state(dev);
			if (ret < 0)
				printf("Error %d\n", ret);
		} else {
			printf("<inactive>\n");
		}
	}

	return 0;
}

int do_button(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const char *button_label;
	struct udevice *dev;
	int ret;

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;
	button_label = argv[1];
	if (strncmp(button_label, "list", 4) == 0)
		return list_buttons();

	ret = button_get_by_label(button_label, &dev);
	if (ret) {
		printf("Button '%s' not found (err=%d)\n", button_label, ret);
		return CMD_RET_FAILURE;
	}

	ret = show_button_state(dev);

	return !ret;
}

U_BOOT_CMD(
	button, 2, 1, do_button,
	"manage buttons",
	"<button_label> \tGet button state\n"
	"button list\t\tShow a list of buttons"
);

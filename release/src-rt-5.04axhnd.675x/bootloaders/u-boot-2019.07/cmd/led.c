// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <led.h>
#include <dm/uclass-internal.h>

#define LED_TOGGLE LEDST_COUNT

static const char *const state_label[] = {
	[LEDST_OFF]	= "off",
	[LEDST_ON]	= "on",
	[LEDST_TOGGLE]	= "toggle",
#ifdef CONFIG_LED_BLINK
	[LEDST_BLINK]	= "blink",
#endif
};

enum led_state_t get_led_cmd(char *var)
{
	int i;

	for (i = 0; i < LEDST_COUNT; i++) {
		if (!strncmp(var, state_label[i], strlen(var)))
			return i;
	}

	return -1;
}

static int show_led_state(struct udevice *dev)
{
	int ret;

	ret = led_get_state(dev);
	if (ret >= LEDST_COUNT)
		ret = -EINVAL;
	if (ret >= 0)
		printf("%s\n", state_label[ret]);

	return ret;
}

static int list_leds(void)
{
	struct udevice *dev;
	int ret;

	for (uclass_find_first_device(UCLASS_LED, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct led_uc_plat *plat = dev_get_uclass_platdata(dev);

		if (!plat->label)
			continue;
		printf("%-15s ", plat->label);
		if (device_active(dev)) {
			ret = show_led_state(dev);
			if (ret < 0)
				printf("Error %d\n", ret);
		} else {
			printf("<inactive>\n");
		}
	}

	return 0;
}

int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum led_state_t cmd;
	const char *led_label;
	struct udevice *dev;
#ifdef CONFIG_LED_BLINK
	int freq_ms = 0;
#endif
	int ret;

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;
	led_label = argv[1];
	if (strncmp(led_label, "list", 4) == 0)
		return list_leds();

	cmd = argc > 2 ? get_led_cmd(argv[2]) : LEDST_COUNT;
#ifdef CONFIG_LED_BLINK
	if (cmd == LEDST_BLINK) {
		if (argc < 4)
			return CMD_RET_USAGE;
		freq_ms = simple_strtoul(argv[3], NULL, 10);
	}
#endif
	ret = led_get_by_label(led_label, &dev);
	if (ret) {
		printf("LED '%s' not found (err=%d)\n", led_label, ret);
		return CMD_RET_FAILURE;
	}
	switch (cmd) {
	case LEDST_OFF:
	case LEDST_ON:
	case LEDST_TOGGLE:
		ret = led_set_state(dev, cmd);
		break;
#ifdef CONFIG_LED_BLINK
	case LEDST_BLINK:
		ret = led_set_period(dev, freq_ms);
		if (!ret)
			ret = led_set_state(dev, LEDST_BLINK);
		break;
#endif
	case LEDST_COUNT:
		printf("LED '%s': ", led_label);
		ret = show_led_state(dev);
		break;
	}
	if (ret < 0) {
		printf("LED '%s' operation failed (err=%d)\n", led_label, ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

#ifdef CONFIG_LED_BLINK
#define BLINK "|blink [blink-freq in ms]"
#else
#define BLINK ""
#endif

U_BOOT_CMD(
	led, 4, 1, do_led,
	"manage LEDs",
	"<led_label> on|off|toggle" BLINK "\tChange LED state\n"
	"led <led_label>\tGet LED state\n"
	"led list\t\tshow a list of LEDs"
);

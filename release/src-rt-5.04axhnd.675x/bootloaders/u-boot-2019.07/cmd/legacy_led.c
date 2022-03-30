// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * Based on cmd_led.c patch from:
 * http://www.mail-archive.com/u-boot@lists.denx.de/msg06873.html
 * (C) Copyright 2008
 * Ulf Samuelsson <ulf.samuelsson@atmel.com>
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <status_led.h>

struct led_tbl_s {
	char		*string;	/* String for use in the command */
	led_id_t	mask;		/* Mask used for calling __led_set() */
	void		(*off)(void);	/* Optional function for turning LED off */
	void		(*on)(void);	/* Optional function for turning LED on */
	void		(*toggle)(void);/* Optional function for toggling LED */
};

typedef struct led_tbl_s led_tbl_t;

static const led_tbl_t led_commands[] = {
#ifdef CONFIG_LED_STATUS_BOARD_SPECIFIC
#ifdef CONFIG_LED_STATUS0
	{ "0", CONFIG_LED_STATUS_BIT, NULL, NULL, NULL },
#endif
#ifdef CONFIG_LED_STATUS1
	{ "1", CONFIG_LED_STATUS_BIT1, NULL, NULL, NULL },
#endif
#ifdef CONFIG_LED_STATUS2
	{ "2", CONFIG_LED_STATUS_BIT2, NULL, NULL, NULL },
#endif
#ifdef CONFIG_LED_STATUS3
	{ "3", CONFIG_LED_STATUS_BIT3, NULL, NULL, NULL },
#endif
#ifdef CONFIG_LED_STATUS4
	{ "4", CONFIG_LED_STATUS_BIT4, NULL, NULL, NULL },
#endif
#ifdef CONFIG_LED_STATUS5
	{ "5", CONFIG_LED_STATUS_BIT5, NULL, NULL, NULL },
#endif
#endif
#ifdef CONFIG_LED_STATUS_GREEN
	{ "green", CONFIG_LED_STATUS_GREEN, green_led_off, green_led_on, NULL },
#endif
#ifdef CONFIG_LED_STATUS_YELLOW
	{ "yellow", CONFIG_LED_STATUS_YELLOW, yellow_led_off, yellow_led_on,
	  NULL },
#endif
#ifdef CONFIG_LED_STATUS_RED
	{ "red", CONFIG_LED_STATUS_RED, red_led_off, red_led_on, NULL },
#endif
#ifdef CONFIG_LED_STATUS_BLUE
	{ "blue", CONFIG_LED_STATUS_BLUE, blue_led_off, blue_led_on, NULL },
#endif
	{ NULL, 0, NULL, NULL, NULL }
};

enum led_cmd { LED_ON, LED_OFF, LED_TOGGLE, LED_BLINK };

enum led_cmd get_led_cmd(char *var)
{
	if (strcmp(var, "off") == 0)
		return LED_OFF;
	if (strcmp(var, "on") == 0)
		return LED_ON;
	if (strcmp(var, "toggle") == 0)
		return LED_TOGGLE;
	if (strcmp(var, "blink") == 0)
		return LED_BLINK;

	return -1;
}

/*
 * LED drivers providing a blinking LED functionality, like the
 * PCA9551, can override this empty weak function
 */
void __weak __led_blink(led_id_t mask, int freq)
{
}

int do_legacy_led(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, match = 0;
	enum led_cmd cmd;
	int freq;

	/* Validate arguments */
	if ((argc < 3) || (argc > 4))
		return CMD_RET_USAGE;

	cmd = get_led_cmd(argv[2]);
	if (cmd < 0) {
		return CMD_RET_USAGE;
	}

	for (i = 0; led_commands[i].string; i++) {
		if ((strcmp("all", argv[1]) == 0) ||
		    (strcmp(led_commands[i].string, argv[1]) == 0)) {
			match = 1;
			switch (cmd) {
			case LED_ON:
				if (led_commands[i].on)
					led_commands[i].on();
				else
					__led_set(led_commands[i].mask,
							  CONFIG_LED_STATUS_ON);
				break;
			case LED_OFF:
				if (led_commands[i].off)
					led_commands[i].off();
				else
					__led_set(led_commands[i].mask,
						  CONFIG_LED_STATUS_OFF);
				break;
			case LED_TOGGLE:
				if (led_commands[i].toggle)
					led_commands[i].toggle();
				else
					__led_toggle(led_commands[i].mask);
				break;
			case LED_BLINK:
				if (argc != 4)
					return CMD_RET_USAGE;

				freq = simple_strtoul(argv[3], NULL, 10);
				__led_blink(led_commands[i].mask, freq);
			}
			/* Need to set only 1 led if led_name wasn't 'all' */
			if (strcmp("all", argv[1]) != 0)
				break;
		}
	}

	/* If we ran out of matches, print Usage */
	if (!match) {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	led, 4, 1, do_legacy_led,
	"["
#ifdef CONFIG_LED_STATUS_BOARD_SPECIFIC
#ifdef CONFIG_LED_STATUS0
	"0|"
#endif
#ifdef CONFIG_LED_STATUS1
	"1|"
#endif
#ifdef CONFIG_LED_STATUS2
	"2|"
#endif
#ifdef CONFIG_LED_STATUS3
	"3|"
#endif
#ifdef CONFIG_LED_STATUS4
	"4|"
#endif
#ifdef CONFIG_LED_STATUS5
	"5|"
#endif
#endif
#ifdef CONFIG_LED_STATUS_GREEN
	"green|"
#endif
#ifdef CONFIG_LED_STATUS_YELLOW
	"yellow|"
#endif
#ifdef CONFIG_LED_STATUS_RED
	"red|"
#endif
#ifdef CONFIG_LED_STATUS_BLUE
	"blue|"
#endif
	"all] [on|off|toggle|blink] [blink-freq in ms]",
	"[led_name] [on|off|toggle|blink] sets or clears led(s)"
);

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>

#include "display.h"

static char *saved_prompt = NULL;
static int saved_point = 0;
static rl_prompt_input_func saved_func = NULL;
static void *saved_user_data = NULL;

void rl_printf(const char *fmt, ...)
{
	va_list args;
	bool save_input;
	char *saved_line;
	int saved_point;

	save_input = !RL_ISSTATE(RL_STATE_DONE);

	if (save_input) {
		saved_point = rl_point;
		saved_line = rl_copy_text(0, rl_end);
		rl_save_prompt();
		rl_replace_line("", 0);
		rl_redisplay();
	}

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	if (save_input) {
		rl_restore_prompt();
		rl_replace_line(saved_line, 0);
		rl_point = saved_point;
		rl_forced_update_display();
		free(saved_line);
	}
}

void rl_hexdump(const unsigned char *buf, size_t len)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[68];
	size_t i;

	if (!len)
		return;

	str[0] = ' ';

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3) + 1] = ' ';
		str[((i % 16) * 3) + 2] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 3] = hexdigits[buf[i] & 0xf];
		str[(i % 16) + 51] = isprint(buf[i]) ? buf[i] : '.';

		if ((i + 1) % 16 == 0) {
			str[49] = ' ';
			str[50] = ' ';
			str[67] = '\0';
			rl_printf("%s\n", str);
			str[0] = ' ';
		}
	}

	if (i % 16 > 0) {
		size_t j;
		for (j = (i % 16); j < 16; j++) {
			str[(j * 3) + 1] = ' ';
			str[(j * 3) + 2] = ' ';
			str[(j * 3) + 3] = ' ';
			str[j + 51] = ' ';
		}
		str[49] = ' ';
		str[50] = ' ';
		str[67] = '\0';
		rl_printf("%s\n", str);
	}
}

void rl_prompt_input(const char *label, const char *msg,
				rl_prompt_input_func func, void *user_data)
{
	char prompt[256];

	/* Normal use should not prompt for user input to the value a second
	 * time before it releases the prompt, but we take a safe action. */
	if (saved_prompt)
		return;

	saved_point = rl_point;
	saved_prompt = strdup(rl_prompt);
	saved_func = func;
	saved_user_data = user_data;

	rl_set_prompt("");
	rl_redisplay();

	memset(prompt, 0, sizeof(prompt));
	snprintf(prompt, sizeof(prompt), COLOR_RED "[%s]" COLOR_OFF " %s ",
								label, msg);
	rl_set_prompt(prompt);

	rl_replace_line("", 0);
	rl_redisplay();
}

int rl_release_prompt(const char *input)
{
	rl_prompt_input_func func;
	void *user_data;

	if (!saved_prompt)
		return -1;

	/* This will cause rl_expand_prompt to re-run over the last prompt, but
	 * our prompt doesn't expand anyway. */
	rl_set_prompt(saved_prompt);
	rl_replace_line("", 0);
	rl_point = saved_point;
	rl_redisplay();

	free(saved_prompt);
	saved_prompt = NULL;

	func = saved_func;
	user_data = saved_user_data;

	saved_func = NULL;
	saved_user_data = NULL;

	func(input, user_data);

	return 0;
}

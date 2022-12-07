/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 */

#define COLOR_OFF	"\x1B[0m"
#define COLOR_RED	"\x1B[0;91m"
#define COLOR_GREEN	"\x1B[0;92m"
#define COLOR_YELLOW	"\x1B[0;93m"
#define COLOR_BLUE	"\x1B[0;94m"
#define COLOR_BOLDGRAY	"\x1B[1;30m"
#define COLOR_BOLDWHITE	"\x1B[1;37m"

void rl_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void rl_hexdump(const unsigned char *buf, size_t len);

typedef void (*rl_prompt_input_func) (const char *input, void *user_data);
void rl_prompt_input(const char *label, const char *msg,
				rl_prompt_input_func func, void *user_data);
int rl_release_prompt(const char *input);

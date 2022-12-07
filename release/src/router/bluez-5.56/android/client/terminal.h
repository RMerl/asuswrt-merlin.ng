/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

#include <stdarg.h>

/* size of supported line */
#define LINE_BUF_MAX 1024

enum key_codes {
	KEY_BACKSPACE = 0x7F,
	KEY_INSERT = 1000, /* arbitrary value */
	KEY_DELETE,
	KEY_HOME,
	KEY_END,
	KEY_PGUP,
	KEY_PGDOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_CLEFT,
	KEY_CRIGHT,
	KEY_CUP,
	KEY_CDOWN,
	KEY_SLEFT,
	KEY_SRIGHT,
	KEY_SUP,
	KEY_SDOWN,
	KEY_MLEFT,
	KEY_MRIGHT,
	KEY_MUP,
	KEY_MDOWN,
	KEY_STAB,
	KEY_M_p,
	KEY_M_n
};

typedef void (*line_callback)(char *);

void terminal_setup(void);
int terminal_print(const char *format, ...);
int terminal_vprint(const char *format, va_list args);
void terminal_process_char(int c, line_callback process_line);
void terminal_insert_into_command_line(const char *p);
void terminal_draw_command_line(void);
void terminal_prompt_for(const char *s, line_callback process_line);

void process_tab(const char *line, int len);

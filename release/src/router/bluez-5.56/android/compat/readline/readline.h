/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 1987-2011 Free Software Foundation, Inc.
 *
 *
 */

#ifndef _READLINE_H_
#define _READLINE_H_

typedef void (*rl_vcpfunc_t)(char *c);
typedef void (*rl_compdisp_func_t)(char **c, int i, int j);
typedef char *(*rl_compentry_func_t)(const char *c, int i);
typedef char **(*rl_completion_func_t)(const char *c, int i, int j);

#define RL_STATE_DONE 0x1000000
#define RL_ISSTATE(x) (rl_readline_state & (x))

static int rl_end;
static int rl_point;
static int rl_readline_state;
static int rl_erase_empty_line;
static int rl_attempted_completion_over;
static char *rl_prompt;
static char *rl_line_buffer;
static rl_compdisp_func_t rl_completion_display_matches_hook;
static rl_completion_func_t rl_attempted_completion_function;

static inline void rl_callback_handler_install(const char *c, rl_vcpfunc_t f)
{
	printf("readline not available\n");
	exit(1);
}

static inline int rl_set_prompt(const char *c)
{
	return -1;
}

static inline void rl_replace_line(const char *c, int i)
{
}

static inline void rl_redisplay(void)
{
}

static inline char **rl_completion_matches(const char *c, rl_compentry_func_t f)
{
	return NULL;
}

static inline int rl_insert_text(const char *c)
{
	return -1;
}

static inline int rl_crlf(void)
{
	return -1;
}

static inline void rl_callback_read_char(void)
{
}

static inline int rl_message(const char *c, ...)
{
	return -1;
}

static inline void rl_callback_handler_remove(void)
{
}

static inline char *rl_copy_text(int i, int j)
{
	return NULL;
}

static inline void rl_save_prompt(void)
{
}

static inline void rl_restore_prompt(void)
{
}

static inline int rl_forced_update_display(void)
{
	return -1;
}

#endif

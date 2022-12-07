/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */
#include <getopt.h>

#define COLOR_OFF	"\001\x1B[0m\002"
#define COLOR_RED	"\001\x1B[0;91m\002"
#define COLOR_GREEN	"\001\x1B[0;92m\002"
#define COLOR_YELLOW	"\001\x1B[0;93m\002"
#define COLOR_BLUE	"\001\x1B[0;94m\002"
#define COLOR_BOLDGRAY	"\001\x1B[1;30m\002"
#define COLOR_BOLDWHITE	"\001\x1B[1;37m\002"
#define COLOR_HIGHLIGHT	"\001\x1B[1;39m\002"

struct bt_shell_menu;

typedef void (*bt_shell_menu_cb_t)(int argc, char *argv[]);
typedef char * (*bt_shell_menu_gen_t)(const char *text, int state);
typedef void (*bt_shell_menu_disp_t) (char **matches, int num_matches,
							int max_length);
typedef void (*bt_shell_prompt_input_func) (const char *input, void *user_data);
typedef bool (*bt_shell_menu_exists_t) (const struct bt_shell_menu *menu);

struct bt_shell_menu_entry {
	const char *cmd;
	const char *arg;
	bt_shell_menu_cb_t func;
	const char *desc;
	bt_shell_menu_gen_t gen;
	bt_shell_menu_disp_t disp;
	bt_shell_menu_exists_t exists;
};

struct bt_shell_menu {
	const char *name;
	const char *desc;
	const struct bt_shell_menu_entry entries[];
};

struct bt_shell_opt {
	const struct option *options;
	size_t optno;
	const char *optstr;
	const char ***optarg;
	const char **help;
};

void bt_shell_init(int argc, char **argv, const struct bt_shell_opt *opt);

int bt_shell_run(void);

void bt_shell_quit(int status);
void bt_shell_noninteractive_quit(int status);

bool bt_shell_set_menu(const struct bt_shell_menu *menu);

bool bt_shell_add_submenu(const struct bt_shell_menu *menu);

bool bt_shell_remove_submenu(const struct bt_shell_menu *menu);

void bt_shell_set_prompt(const char *string);

void bt_shell_printf(const char *fmt,
				...) __attribute__((format(printf, 1, 2)));
void bt_shell_hexdump(const unsigned char *buf, size_t len);
void bt_shell_usage(void);

void bt_shell_prompt_input(const char *label, const char *msg,
			bt_shell_prompt_input_func func, void *user_data);
int bt_shell_release_prompt(const char *input);

bool bt_shell_attach(int fd);
bool bt_shell_detach(void);

void bt_shell_set_env(const char *name, void *value);
void *bt_shell_get_env(const char *name);

void bt_shell_cleanup(void);

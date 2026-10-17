// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Caleb Connolly <caleb.connolly@linaro.org>
 */

#include <button.h>
#include <command.h>
#include <env.h>
#include <log.h>
#include <vsprintf.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

struct button_cmd {
	bool pressed;
	const char *btn_name;
	const char *cmd;
};

/*
 * Button commands are set via environment variables, e.g.:
 * button_cmd_N_name=Volume Up
 * button_cmd_N=fastboot usb 0
 *
 * This function will retrieve the command for the given button N
 * and populate the cmd struct with the command string and pressed
 * state of the button.
 *
 * Returns 1 if a command was found, 0 otherwise.
 */
static int get_button_cmd(int n, struct button_cmd *cmd)
{
	const char *cmd_str;
	struct udevice *btn;
	char buf[24];

	snprintf(buf, sizeof(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	if (!cmd->btn_name)
		return 0;

	button_get_by_label(cmd->btn_name, &btn);
	if (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		return 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	if (!cmd->pressed)
		return 1;

	snprintf(buf, sizeof(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	if (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		return 0;
	}

	cmd->cmd = cmd_str;

	return 1;
}

void process_button_cmds(void)
{
	struct button_cmd cmd = {0};
	int i = 0;

	while (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		if (!cmd.pressed)
			continue;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		return;
	}
}

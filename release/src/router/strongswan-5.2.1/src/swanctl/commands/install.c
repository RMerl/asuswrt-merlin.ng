/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "command.h"

#include <errno.h>

static int manage_policy(vici_conn_t *conn, char *label)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *child = NULL;
	int ret = 0;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'P':
				format |= COMMAND_FORMAT_RAW;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_PRETTY;
				continue;
			case 'c':
				child = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --%s option", label);
		}
		break;
	}
	req = vici_begin(label);
	if (child)
	{
		vici_add_key_valuef(req, "child", "%s", child);
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "%s request failed: %s\n", label, strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		puts(label);
		vici_dump(res, " reply", format & COMMAND_FORMAT_PRETTY, stdout);
	}
	else
	{
		if (streq(vici_find_str(res, "no", "success"), "yes"))
		{
			printf("%s completed successfully\n", label);
		}
		else
		{
			fprintf(stderr, "%s failed: %s\n",
					label, vici_find_str(res, "", "errmsg"));
			ret = 1;
		}
	}
	vici_free_res(res);
	return ret;
}

static int uninstall(vici_conn_t *conn)
{
	return manage_policy(conn, "uninstall");
}

static int install(vici_conn_t *conn)
{
	return manage_policy(conn, "install");
}

/**
 * Register the uninstall command.
 */
static void __attribute__ ((constructor))reg_uninstall()
{
	command_register((command_t) {
		uninstall, 'u', "uninstall", "uninstall a trap or shunt policy",
		{"--child <name> [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"child",		'c', 1, "CHILD_SA configuration to uninstall"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

/**
 * Register install the command.
 */
static void __attribute__ ((constructor))reg_install()
{
	command_register((command_t) {
		install, 'p', "install", "install a trap or shunt policy",
		{"--child <name> [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"child",		'c', 1, "CHILD_SA configuration to install"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

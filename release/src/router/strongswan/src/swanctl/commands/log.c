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
#include <unistd.h>

CALLBACK(log_cb, void,
	command_format_options_t *format, char *name, vici_res_t *msg)
{
	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(msg, "log", *format & COMMAND_FORMAT_PRETTY, stdout);
	}
	else
	{
		char *current, *next;

		current = vici_find_str(msg, NULL, "msg");
		while (current)
		{
			next = strchr(current, '\n');
			printf("%.2d[%s] ", vici_find_int(msg, 0, "thread"),
				   vici_find_str(msg, "   ", "group"));
			if (next == NULL)
			{
				printf("%s\n", current);
				break;
			}
			printf("%.*s\n", (int)(next - current), current);
			current = next + 1;
		}
	}
}

static int logcmd(vici_conn_t *conn)
{
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --log option");
		}
		break;
	}

	if (vici_register(conn, "log", log_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for log failed: %s\n", strerror(errno));
		return ret;
	}

	wait_sigint();

	fprintf(stderr, "disconnecting...\n");

	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		logcmd, 'T', "log", "trace logging output",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

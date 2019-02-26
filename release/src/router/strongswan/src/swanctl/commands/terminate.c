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

CALLBACK(log_cb, void,
	command_format_options_t *format, char *name, vici_res_t *msg)
{
	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(msg, "log", *format & COMMAND_FORMAT_PRETTY, stdout);
	}
	else
	{
		printf("[%s] %s\n",
			   vici_find_str(msg, "   ", "group"),
			   vici_find_str(msg, "", "msg"));
	}
}

static int terminate(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *child = NULL, *ike = NULL;
	int ret = 0, timeout = 0, level = 1, child_id = 0, ike_id = 0;
	bool force = FALSE;

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
			case 'c':
				child = arg;
				continue;
			case 'f':
				force = TRUE;
				continue;
			case 'i':
				ike = arg;
				continue;
			case 'C':
				child_id = atoi(arg);
				continue;
			case 'I':
				ike_id = atoi(arg);
				continue;
			case 't':
				timeout = atoi(arg);
				continue;
			case 'l':
				level = atoi(arg);
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --terminate option");
		}
		break;
	}

	if (vici_register(conn, "control-log", log_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for log failed: %s\n", strerror(errno));
		return ret;
	}
	req = vici_begin("terminate");
	if (child)
	{
		vici_add_key_valuef(req, "child", "%s", child);
	}
	if (ike)
	{
		vici_add_key_valuef(req, "ike", "%s", ike);
	}
	if (child_id)
	{
		vici_add_key_valuef(req, "child-id", "%d", child_id);
	}
	if (ike_id)
	{
		vici_add_key_valuef(req, "ike-id", "%d", ike_id);
	}
	if (force)
	{
		vici_add_key_valuef(req, "force", "yes");
	}
	if (timeout)
	{
		vici_add_key_valuef(req, "timeout", "%d", timeout * 1000);
	}
	vici_add_key_valuef(req, "loglevel", "%d", level);
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "terminate request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "terminate reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (streq(vici_find_str(res, "no", "success"), "yes"))
		{
			printf("terminate completed successfully\n");
		}
		else
		{
			fprintf(stderr, "terminate failed: %s\n",
					vici_find_str(res, "", "errmsg"));
			ret = 1;
		}
	}
	vici_free_res(res);
	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		terminate, 't', "terminate", "terminate a connection",
		{"--child <name> | --ike <name> | --child-id <id> | --ike-id <id>",
		 "[--timeout <s>] [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"child",		'c', 1, "terminate by CHILD_SA name"},
			{"ike",			'i', 1, "terminate by IKE_SA name"},
			{"child-id",	'C', 1, "terminate by CHILD_SA reqid"},
			{"ike-id",		'I', 1, "terminate by IKE_SA unique identifier"},
			{"force",		'f', 0, "terminate IKE_SA without waiting, unless timeout is set"},
			{"timeout",		't', 1, "timeout in seconds before detaching"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"loglevel",	'l', 1, "verbosity of redirected log"},
		}
	});
}

/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "command.h"

CALLBACK(counters_kv, int,
	void *null, vici_res_t *res, char *name, void *value, int len)
{
	if (chunk_printable(chunk_create(value, len), NULL, ' '))
	{
		printf("  %-22s: %.*s\n", name, len, value);
	}
	return 0;
}

CALLBACK(conns_sn, int,
	void *null, vici_res_t *res, char *name)
{
	printf("%s:\n", strlen(name) ? name : "global");
	return vici_parse_cb(res, NULL, counters_kv, NULL, NULL);
}

CALLBACK(counters_sn, int,
	void *null, vici_res_t *res, char *name)
{
	return vici_parse_cb(res, conns_sn, NULL, NULL, NULL);
}

static int counters(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *name = NULL;
	int ret = 0;
	bool all = FALSE, reset = FALSE;

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
			case 'n':
				name = arg;
				continue;
			case 'a':
				all = TRUE;
				continue;
			case 'R':
				reset = TRUE;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --counters option");
		}
		break;
	}
	if (reset)
	{
		req = vici_begin("reset-counters");
	}
	else
	{
		req = vici_begin("get-counters");
	}
	if (all)
	{
		vici_add_key_valuef(req, "all", "yes");
	}
	else if (name)
	{
		vici_add_key_valuef(req, "name", "%s", name);
	}

	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "%s-counters request failed: %s\n",
				reset ? "reset" : "get", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "counters reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (streq(vici_find_str(res, "no", "success"), "yes"))
		{
			if (reset)
			{
				printf("reset-counters completed successfully\n");
			}
			else if (vici_parse_cb(res, counters_sn, NULL, NULL, NULL) != 0)
			{
				fprintf(stderr, "parsing get-counters reply failed: %s\n",
						strerror(errno));
			}
		}
		else
		{
			fprintf(stderr, "%s-counters failed: %s\n", reset ? "reset" : "get",
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
		counters, 'C', "counters", "list or reset IKE event counters",
		{"[--name <name>|--all] [--reset] [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"name",		'n', 1, "connection name, omit for global counters"},
			{"all",			'a', 0, "get/reset counters for all tracked connections"},
			{"reset",		'R', 0, "reset the counters"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

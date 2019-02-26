/*
 * Copyright (C) 2015-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "command.h"

CALLBACK(list_leases, int,
	char *pool, vici_res_t *res, char *name)
{
	if (streq(name, "leases"))
	{
		return vici_parse_cb(res, list_leases, NULL, NULL, pool);
	}
	printf("  %-30s %-8s '%s'\n",
		   vici_find_str(res, "", "%s.leases.%s.address", pool, name),
		   vici_find_str(res, "", "%s.leases.%s.status", pool, name),
		   vici_find_str(res, "", "%s.leases.%s.identity", pool, name));
	return 0;
}

CALLBACK(list_pool, int,
	void *not_used, vici_res_t *res, char *name)
{
	char pool[64], leases[32];

	snprintf(pool, sizeof(pool), "%s:", name);
	snprintf(leases, sizeof(leases), "%s / %s / %s",
		vici_find_str(res, "", "%s.online", name),
		vici_find_str(res, "", "%s.offline", name),
		vici_find_str(res, "", "%s.size", name));

	printf("%-20s %-30s %16s\n",
		name, vici_find_str(res, "", "%s.base", name), leases);

	return vici_parse_cb(res, list_leases, NULL, NULL, name);
}

static int list_pools(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *name = NULL;
	int ret = 0;
	bool leases = FALSE;

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
			case 'l':
				leases = TRUE;
				continue;
			case 'n':
				name = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --list-pools option");
		}
		break;
	}

	req = vici_begin("get-pools");
	if (leases)
	{
		vici_add_key_valuef(req, "leases", "yes");
	}
	if (name)
	{
		vici_add_key_valuef(req, "name", "%s", name);
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "get-pools request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "get-pools reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		ret = vici_parse_cb(res, list_pool, NULL, NULL, NULL);
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
		list_pools, 'A', "list-pools", "list loaded pool configurations",
		{"[--leases] [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
			{"leases",		'l', 0, "list leases of each pool"},
			{"name",		'n', 1, "filter pools by name"},
		}
	});
}

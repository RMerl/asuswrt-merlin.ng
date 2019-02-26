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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "command.h"

#include <collections/hashtable.h>

/**
 * Free hashtable with contained strings
 */
static void free_hashtable(hashtable_t *hashtable)
{
	enumerator_t *enumerator;
	char *str;

	enumerator = hashtable->create_enumerator(hashtable);
	while (enumerator->enumerate(enumerator, NULL, &str))
	{
		free(str);
	}
	enumerator->destroy(enumerator);

	hashtable->destroy(hashtable);
}

CALLBACK(policy_values, int,
	hashtable_t *pol, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;
	char *str;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		if (asprintf(&str, "%.*s", len, value) >= 0)
		{
			free(pol->put(pol, name, str));
		}
	}
	return 0;
}

CALLBACK(policy_list, int,
	hashtable_t *pol, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;
	char *str;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		str = pol->get(pol, name);
		if (asprintf(&str, "%s%s%.*s",
					 str ?: "", str ? " " : "", len, value) >= 0)
		{
			free(pol->put(pol, name, str));
		}
	}
	return 0;
}

CALLBACK(policies, int,
	void *null, vici_res_t *res, char *name)
{
	hashtable_t *pol;
	int ret;

	pol = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	ret = vici_parse_cb(res, NULL, policy_values, policy_list, pol);

	printf("%s, %s\n", name, pol->get(pol, "mode"));
	printf("  local:  %s\n", pol->get(pol, "local-ts"));
	printf("  remote: %s\n", pol->get(pol, "remote-ts"));

	free_hashtable(pol);
	return ret;
}

CALLBACK(list_cb, void,
	command_format_options_t *format, char *name, vici_res_t *res)
{
	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-policy event", *format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (vici_parse_cb(res, policies, NULL, NULL, NULL) != 0)
		{
			fprintf(stderr, "parsing policy event failed: %s\n", strerror(errno));
		}
	}
}

static int list_pols(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	bool trap = FALSE, drop = FALSE, pass = FALSE;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *child = NULL;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'c':
				child = arg;
				continue;
			case 't':
				trap = TRUE;
				continue;
			case 'd':
				drop = TRUE;
				continue;
			case 'p':
				pass = TRUE;
				continue;
			case 'P':
				format |= COMMAND_FORMAT_PRETTY;
				/* fall through to raw */
			case 'r':
				format |= COMMAND_FORMAT_RAW;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --list-pols option");
		}
		break;
	}
	if (!trap && !drop && !pass)
	{
		trap = drop = pass = TRUE;
	}
	if (vici_register(conn, "list-policy", list_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for policies failed: %s\n",
				strerror(errno));
		return ret;
	}
	req = vici_begin("list-policies");
	if (child)
	{
		vici_add_key_valuef(req, "child", "%s", child);
	}
	if (trap)
	{
		vici_add_key_valuef(req, "trap", "yes");
	}
	if (drop)
	{
		vici_add_key_valuef(req, "drop", "yes");
	}
	if (pass)
	{
		vici_add_key_valuef(req, "pass", "yes");
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "list-policies request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-policies reply", format & COMMAND_FORMAT_PRETTY, stdout);
	}
	vici_free_res(res);
	return 0;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		list_pols, 'P', "list-pols", "list currently installed policies",
		{"[--child <name>] [--trap] [--drop] [--pass] [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"child",		'c', 1, "filter policies by CHILD_SA config name"},
			{"trap",		't', 0, "list trap policies"},
			{"drop",		'd', 0, "list drop policies"},
			{"pass",		'p', 0, "list bypass policies"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

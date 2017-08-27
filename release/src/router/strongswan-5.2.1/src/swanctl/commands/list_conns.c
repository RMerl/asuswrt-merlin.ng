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

CALLBACK(values, int,
	hashtable_t *sa, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;
	char *str;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		if (asprintf(&str, "%.*s", len, value) >= 0)
		{
			free(sa->put(sa, name, str));
		}
	}
	return 0;
}


CALLBACK(list, int,
	hashtable_t *sa, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;
	char *str;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		str = sa->get(sa, name);
		if (asprintf(&str, "%s%s%.*s",
					 str ?: "", str ? " " : "", len, value) >= 0)
		{
			free(sa->put(sa, name, str));
		}
	}
	return 0;
}

CALLBACK(children_sn, int,
	hashtable_t *ike, vici_res_t *res, char *name)
{
	hashtable_t *child;
	int ret;

	child = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	ret = vici_parse_cb(res, NULL, values, list, child);
	if (ret == 0)
	{
		printf("  %s: %s\n", name, child->get(child, "mode"));
		printf("    local:  %s\n", child->get(child, "local-ts"));
		printf("    remote: %s\n", child->get(child, "remote-ts"));
	}
	free_hashtable(child);
	return ret;
}

CALLBACK(conn_sn, int,
	hashtable_t *ike, vici_res_t *res, char *name)
{
	int ret = 0;

	if (streq(name, "children"))
	{
		return vici_parse_cb(res, children_sn, NULL, NULL, NULL);
	}
	if (streq(name, "local") || streq(name, "remote"))
	{
		hashtable_t *auth;

		auth = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
		ret = vici_parse_cb(res, NULL, values, list, auth);
		if (ret == 0)
		{
			printf("  %s %s authentication:\n",
				name, auth->get(auth, "class") ?: "unspecified");
			if (auth->get(auth, "id"))
			{
				printf("    id: %s\n", auth->get(auth, "id"));
			}
			if (auth->get(auth, "groups"))
			{
				printf("    groups: %s\n", auth->get(auth, "groups"));
			}
			if (auth->get(auth, "certs"))
			{
				printf("    certs: %s\n", auth->get(auth, "certs"));
			}
			if (auth->get(auth, "cacerts"))
			{
				printf("    cacerts: %s\n", auth->get(auth, "cacerts"));
			}
		}
		free_hashtable(auth);
	}
	return ret;
}

CALLBACK(conn_list, int,
	hashtable_t *sa, vici_res_t *res, char *name, void *value, int len)
{
	if (chunk_printable(chunk_create(value, len), NULL, ' '))
	{
		if (streq(name, "local_addrs"))
		{
			printf("  local:  %.*s\n", len, value);
		}
		if (streq(name, "remote_addrs"))
		{
			printf("  remote: %.*s\n", len, value);
		}
	}
	return 0;
}

CALLBACK(conns, int,
	void *null, vici_res_t *res, char *name)
{
	printf("%s: %s\n", name, vici_find_str(res, "", "%s.version", name));

	return vici_parse_cb(res, conn_sn, NULL, conn_list, NULL);
}

CALLBACK(list_cb, void,
	command_format_options_t *format, char *name, vici_res_t *res)
{
	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-conn event", *format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (vici_parse_cb(res, conns, NULL, NULL, NULL) != 0)
		{
			fprintf(stderr, "parsing conn event failed: %s\n", strerror(errno));
		}
	}
}

static int list_conns(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
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
				return command_usage("invalid --list-conns option");
		}
		break;
	}
	if (vici_register(conn, "list-conn", list_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for connections failed: %s\n",
				strerror(errno));
		return ret;
	}
	req = vici_begin("list-conns");
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "list-conns request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-conns reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
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
		list_conns, 'L', "list-conns", "list loaded configurations",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

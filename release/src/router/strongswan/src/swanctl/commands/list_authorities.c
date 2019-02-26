/*
 * Copyright (C) 2015 Andreas Steffen
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

#define LABELED_CRL_URI		(1 << 0)
#define LABELED_OCSP_URI	(1 << 1)

CALLBACK(authority_kv, int,
	void *null, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		printf("  %s: %.*s\n", name, len, value);
	}

	return 0;
}


CALLBACK(authority_list, int,
	int *labeled, vici_res_t *res, char *name, void *value, int len)
{
	chunk_t chunk;

	chunk = chunk_create(value, len);
	if (chunk_printable(chunk, NULL, ' '))
	{
		if (streq(name, "crl_uris"))
		{
			printf("  %s %.*s\n",
				  (*labeled & LABELED_CRL_URI)  ? "          " : "crl_uris: ", 
				  len, value);
			*labeled |= LABELED_CRL_URI;
		}
		if (streq(name, "ocsp_uris"))
		{
			printf("  %s %.*s\n",
				  (*labeled & LABELED_OCSP_URI) ? "          " : "ocsp_uris:",
				  len, value);
			*labeled %= LABELED_OCSP_URI;
		}
	}
	return 0;
}

CALLBACK(authorities, int,
	void *null, vici_res_t *res, char *name)
{
	int labeled = 0;

	printf("%s:\n", name);

	return vici_parse_cb(res, NULL, authority_kv, authority_list, &labeled);
}

CALLBACK(list_cb, void,
	command_format_options_t *format, char *name, vici_res_t *res)
{
	if (*format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-authorities event", *format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (vici_parse_cb(res, authorities, NULL, NULL, NULL) != 0)
		{
			fprintf(stderr, "parsing authority event failed: %s\n",
					strerror(errno));
		}
	}
}

static int list_authorities(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *ca_name = NULL;;
	int ret = 0;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'n':
				ca_name = arg;
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
				return command_usage("invalid --list-authorities option");
		}
		break;
	}
	if (vici_register(conn, "list-authority", list_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for authorities failed: %s\n",
				strerror(errno));
		return ret;
	}

	req = vici_begin("list-authorities");
	if (ca_name)
	{
		vici_add_key_valuef(req, "name", "%s", ca_name);
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "list-authorities request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-authorities reply", format & COMMAND_FORMAT_PRETTY,
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
		list_authorities, 'B', "list-authorities",
		"list loaded authority configurations",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"name",		'n', 1, "filter by authority name"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

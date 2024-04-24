/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2016 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

/*
 * Copyright (C) 2014 Timo Teräs <timo.teras@iki.fi>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include "command.h"
#include "swanctl.h"

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

CALLBACK(sa_values, int,
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

CALLBACK(sa_list, int,
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

CALLBACK(child_sas, int,
	hashtable_t *ike, vici_res_t *res, char *name)
{
	hashtable_t *child;
	int ret;

	child = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	ret = vici_parse_cb(res, NULL, sa_values, sa_list, child);
	if (ret == 0)
	{
		printf("  %s: #%s, reqid %s, %s, %s%s, %s:",
			child->get(child, "name"), child->get(child, "uniqueid"),
			child->get(child, "reqid"), child->get(child, "state"),
			child->get(child, "mode"),
			child->get(child, "encap") ? "-in-UDP" : "",
			child->get(child, "protocol"));

		if (child->get(child, "encr-alg"))
		{
			printf("%s", child->get(child, "encr-alg"));
			if (child->get(child, "encr-keysize"))
			{
				printf("-%s", child->get(child, "encr-keysize"));
			}
		}
		if (child->get(child, "integ-alg"))
		{
			if (child->get(child, "encr-alg"))
			{
				printf("/");
			}
			printf("%s", child->get(child, "integ-alg"));
			if (child->get(child, "integ-keysize"))
			{
				printf("-%s", child->get(child, "integ-keysize"));
			}
		}
		if (child->get(child, "prf-alg"))
		{
			printf("/%s", child->get(child, "prf-alg"));
		}
		if (child->get(child, "dh-group"))
		{
			printf("/%s", child->get(child, "dh-group"));
		}
		if (child->get(child, "esn"))
		{
			printf("/ESN");
		}
		printf("\n");

		printf("    installed %ss ago", child->get(child, "install-time"));
		if (child->get(child, "rekey-time"))
		{
			printf(", rekeying in %ss", child->get(child, "rekey-time"));
		}
		if (child->get(child, "life-time"))
		{
			printf(", expires in %ss", child->get(child, "life-time"));
		}
		printf("\n");

		printf("    in  %s%s%s", child->get(child, "spi-in"),
			child->get(child, "cpi-in") ? "/" : "",
			child->get(child, "cpi-in") ?: "");
		if (child->get(child, "mark-in") || child->get(child, "if-id-in"))
		{
			printf(" (");
			if (child->get(child, "mark-in"))
			{
				printf("0x%s", child->get(child, "mark-in"));
				if (child->get(child, "mark-mask-in"))
				{
					printf("/0x%s", child->get(child, "mark-mask-in"));
				}
			}
			else
			{
				printf("-");
			}
			if (child->get(child, "if-id-in"))
			{
				printf("|0x%s", child->get(child, "if-id-in"));
			}
			printf(")");
		}
		printf(", %6s bytes, %5s packets",
			child->get(child, "bytes-in"), child->get(child, "packets-in"));
		if (child->get(child, "use-in"))
		{
			printf(", %5ss ago", child->get(child, "use-in"));
		}
		printf("\n");

		printf("    out %s%s%s", child->get(child, "spi-out"),
			child->get(child, "cpi-out") ? "/" : "",
			child->get(child, "cpi-out") ?: "");
		if (child->get(child, "mark-out") || child->get(child, "if-id-out"))
		{
			printf(" (");
			if (child->get(child, "mark-out"))
			{
				printf("0x%s", child->get(child, "mark-out"));
				if (child->get(child, "mark-mask-out"))
				{
					printf("/0x%s", child->get(child, "mark-mask-out"));
				}
			}
			else
			{
				printf("-");
			}
			if (child->get(child, "if-id-out"))
			{
				printf("|0x%s", child->get(child, "if-id-out"));
			}
			printf(")");
		}
		printf(", %6s bytes, %5s packets",
			child->get(child, "bytes-out"), child->get(child, "packets-out"));
		if (child->get(child, "use-out"))
		{
			printf(", %5ss ago", child->get(child, "use-out"));
		}
		printf("\n");

		print_label("    label  ", child->get(child, "label"));
		printf("    local  %s\n", child->get(child, "local-ts"));
		printf("    remote %s\n", child->get(child, "remote-ts"));
	}
	free_hashtable(child);
	return ret;
}

CALLBACK(ike_sa, int,
	hashtable_t *ike, vici_res_t *res, char *name)
{
	if (streq(name, "child-sas"))
	{
		bool is_initiator = streq(ike->get(ike, "initiator"), "yes");

		printf("%s: #%s, %s, IKEv%s, %s_i%s %s_r%s\n",
			ike->get(ike, "name"), ike->get(ike, "uniqueid"),
			ike->get(ike, "state"), ike->get(ike, "version"),
			ike->get(ike, "initiator-spi"), is_initiator ? "*" : "",
			ike->get(ike, "responder-spi"), is_initiator ? "" : "*");

		printf("  local  '%s' @ %s[%s]",
			ike->get(ike, "local-id"), ike->get(ike, "local-host"),
			ike->get(ike, "local-port"));
		if (ike->get(ike, "local-vips"))
		{
			printf(" [%s]", ike->get(ike, "local-vips"));
		}
		printf("\n");

		printf("  remote '%s' @ %s[%s]",
			ike->get(ike, "remote-id"), ike->get(ike, "remote-host"),
			ike->get(ike, "remote-port"));
		if (ike->get(ike, "remote-eap-id"))
		{
			printf(" EAP: '%s'", ike->get(ike, "remote-eap-id"));
		}
		if (ike->get(ike, "remote-xauth-id"))
		{
			printf(" XAuth: '%s'", ike->get(ike, "remote-xauth-id"));
		}
		if (ike->get(ike, "remote-vips"))
		{
			printf(" [%s]", ike->get(ike, "remote-vips"));
		}
		printf("\n");

		if (ike->get(ike, "encr-alg"))
		{
			printf("  %s", ike->get(ike, "encr-alg"));
			if (ike->get(ike, "encr-keysize"))
			{
				printf("-%s", ike->get(ike, "encr-keysize"));
			}
			if (ike->get(ike, "integ-alg"))
			{
				printf("/%s", ike->get(ike, "integ-alg"));
			}
			if (ike->get(ike, "integ-keysize"))
			{
				printf("-%s", ike->get(ike, "integ-keysize"));
			}
			printf("/%s", ike->get(ike, "prf-alg"));
			printf("/%s", ike->get(ike, "dh-group"));
			if (streq(ike->get(ike, "ppk"), "yes"))
			{
				printf("/PPK");
			}
			printf("\n");
		}

		if (ike->get(ike, "established"))
		{
			printf("  established %ss ago", ike->get(ike, "established"));
			if (ike->get(ike, "rekey-time"))
			{
				printf(", rekeying in %ss", ike->get(ike, "rekey-time"));
			}
			if (ike->get(ike, "reauth-time"))
			{
				printf(", reauth in %ss", ike->get(ike, "reauth-time"));
			}
			if (ike->get(ike, "life-time"))
			{
				printf(", expires in %ss", ike->get(ike, "life-time"));
			}
			printf("\n");
		}

		if (ike->get(ike, "tasks-queued"))
		{
			printf("  queued:  %s\n", ike->get(ike, "tasks-queued"));
		}
		if (ike->get(ike, "tasks-active"))
		{
			printf("  active:  %s\n", ike->get(ike, "tasks-active"));
		}
		if (ike->get(ike, "tasks-passive"))
		{
			printf("  passive: %s\n", ike->get(ike, "tasks-passive"));
		}

		return vici_parse_cb(res, child_sas, NULL, NULL, ike);
	}
	return 0;
}

CALLBACK(ike_sas, int,
	void *null, vici_res_t *res, char *name)
{
	hashtable_t *ike;
	int ret;

	ike = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	ike->put(ike, "name", strdup(name));
	ret = vici_parse_cb(res, ike_sa, sa_values, sa_list, ike);
	free_hashtable(ike);
	return ret;
}

CALLBACK(list_cb, void,
	command_format_options_t *format, char *name, vici_res_t *res)
{
	char buf[256];

	if (*format & COMMAND_FORMAT_RAW)
	{
		snprintf(buf, sizeof(buf), "%s event", name);
		vici_dump(res, buf, *format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (vici_parse_cb(res, ike_sas, NULL, NULL, NULL) != 0)
		{
			fprintf(stderr, "parsing SA event failed: %s\n", strerror(errno));
		}
	}
}

static int list_sas(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	bool noblock = FALSE;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *ike = NULL, *child = NULL;
	int ike_id = 0, child_id = 0, ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'i':
				ike = arg;
				continue;
			case 'I':
				ike_id = atoi(arg);
				continue;
			case 'c':
				child = arg;
				continue;
			case 'C':
				child_id = atoi(arg);
				continue;
			case 'n':
				noblock = TRUE;
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
				return command_usage("invalid --list-sas option");
		}
		break;
	}
	if (vici_register(conn, "list-sa", list_cb, &format) != 0)
	{
		ret = errno;
		fprintf(stderr, "registering for SAs failed: %s\n", strerror(errno));
		return ret;
	}
	req = vici_begin("list-sas");
	if (ike)
	{
		vici_add_key_valuef(req, "ike", "%s", ike);
	}
	if (ike_id)
	{
		vici_add_key_valuef(req, "ike-id", "%d", ike_id);
	}
	if (child)
	{
		vici_add_key_valuef(req, "child", "%s", child);
	}
	if (child_id)
	{
		vici_add_key_valuef(req, "child-id", "%d", child_id);
	}
	if (noblock)
	{
		vici_add_key_valuef(req, "noblock", "yes");
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "list-sas request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "list-sas reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	vici_free_res(res);
	return 0;
}

CALLBACK(close_cb, void,
	int *ret)
{
	fprintf(stderr, "connection closed\n");
	*ret = ECONNRESET;
	send_sigint();
}

static int monitor_sas(vici_conn_t *conn)
{
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg;
	int ret = 0;

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
				return command_usage("invalid --monitor-sa option");
		}
		break;
	}

	vici_on_close(conn, close_cb, &ret);

	if (vici_register(conn, "ike-updown", list_cb, &format) != 0)
	{
		fprintf(stderr, "registering for IKE_SAs failed: %s\n",
				strerror(errno));
		return errno;
	}
	if (vici_register(conn, "child-updown", list_cb, &format) != 0)
	{
		fprintf(stderr, "registering for CHILD_SAs failed: %s\n",
				strerror(errno));
		return errno;
	}

	wait_sigint();

	if (!ret)
	{
		fprintf(stderr, "disconnecting...\n");
	}
	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		list_sas, 'l', "list-sas", "list currently active IKE_SAs",
		{"[--ike <name>|--ike-id <id>] [--child <name>|--child-id <id>]",
		 "[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"ike",			'i', 1, "filter IKE_SAs by name"},
			{"ike-id",		'I', 1, "filter IKE_SAs by unique identifier"},
			{"child",		'c', 1, "filter CHILD_SAs by name"},
			{"child-id",	'C', 1, "filter CHILD_SAs by unique identifier"},
			{"noblock",		'n', 0, "don't wait for IKE_SAs in use"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

static void __attribute__ ((constructor))reg_monitor_sa()
{
	command_register((command_t) {
		monitor_sas, 'm', "monitor-sa", "monitor for IKE_SA and CHILD_SA changes",
		{"[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

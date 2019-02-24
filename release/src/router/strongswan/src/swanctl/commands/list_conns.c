/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * Copyright (C) 2016-2018 Andreas Steffen
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
	char *mode, *interface, *priority;
	char *rekey_time, *rekey_bytes, *rekey_packets, *dpd_action, *dpd_delay;
	bool no_time, no_bytes, no_packets, no_dpd, or = FALSE;
	int ret;

	child = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	ret = vici_parse_cb(res, NULL, values, list, child);
	if (ret == 0)
	{
		mode = child->get(child, "mode");
		printf("  %s: %s, ", name, mode);

		rekey_time    = child->get(child, "rekey_time");
		rekey_bytes   = child->get(child, "rekey_bytes");
		rekey_packets = child->get(child, "rekey_packets");
		dpd_action    = child->get(child, "dpd_action");
		dpd_delay     = ike->get(ike, "dpd_delay");

		no_time    = streq(rekey_time, "0");
		no_bytes   = streq(rekey_bytes, "0");
		no_packets = streq(rekey_packets, "0");
		no_dpd     = streq(dpd_delay, "0");

		if (strcaseeq(mode, "PASS") || strcaseeq(mode, "DROP") ||
		   (no_time && no_bytes && no_packets))
		{
			printf("no rekeying");
		}
		else
		{
			printf("rekeying every");
			if (!no_time)
			{
				printf(" %ss", rekey_time);
				or = TRUE;
			}
			if (!no_bytes)
			{
				printf("%s %s bytes", or ? " or" : "", rekey_bytes);
				or = TRUE;
			}
			if (!no_packets)
			{
				printf("%s %s packets", or ? " or" : "", rekey_packets);
			}
		}
		if (!no_dpd)
		{
			printf(", dpd action is %s", dpd_action);
		}
		printf("\n");

		printf("    local:  %s\n", child->get(child, "local-ts"));
		printf("    remote: %s\n", child->get(child, "remote-ts"));

		interface = child->get(child, "interface");
		if (interface)
		{
			printf("    interface: %s\n", interface);
		}

		priority = child->get(child, "priority");
		if (priority)
		{
			printf("    priority: %s\n", priority);
		}
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
		return vici_parse_cb(res, children_sn, NULL, NULL, ike);
	}
	if (strpfx(name, "local") || strpfx(name, "remote"))
	{
		hashtable_t *auth;
		char *class;

		auth = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
		ret = vici_parse_cb(res, NULL, values, list, auth);
		if (ret == 0)
		{
			class = auth->get(auth, "class") ?: "unspecified";
			if (strcaseeq(class, "EAP"))
			{
				class = auth->get(auth, "eap-type") ?: class;
			}
			printf("  %s %s authentication:\n",
				strpfx(name, "local") ? "local" : "remote", class);
			if (auth->get(auth, "id"))
			{
				printf("    id: %s\n", auth->get(auth, "id"));
			}
			if (auth->get(auth, "eap_id"))
			{
				printf("    eap_id: %s\n", auth->get(auth, "eap_id"));
			}
			if (auth->get(auth, "xauth_id"))
			{
				printf("    xauth_id: %s\n", auth->get(auth, "xauth_id"));
			}
			if (auth->get(auth, "aaa_id"))
			{
				printf("    aaa_id: %s\n", auth->get(auth, "aaa_id"));
			}
			if (auth->get(auth, "groups"))
			{
				printf("    groups: %s\n", auth->get(auth, "groups"));
			}
			if (auth->get(auth, "cert_policy"))
			{
				printf("    cert policy: %s\n", auth->get(auth, "cert_policy"));
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
	int ret;
	char *version, *reauth_time, *rekey_time, *dpd_delay, *ppk_id, *ppk_req;
	hashtable_t *ike;

	version     = vici_find_str(res, "", "%s.version", name);
	reauth_time = vici_find_str(res, "0", "%s.reauth_time", name);
	rekey_time  = vici_find_str(res, "0", "%s.rekey_time", name);
	dpd_delay   = vici_find_str(res, "0", "%s.dpd_delay", name);

	ike = hashtable_create(hashtable_hash_str, hashtable_equals_str, 1);
	free(ike->put(ike,"dpd_delay", strdup(dpd_delay)));

	printf("%s: %s, ", name, version);
	if (streq(version, "IKEv1"))
	{
		if (streq(reauth_time, "0"))
		{
			reauth_time = rekey_time;
		}
	}
	if (streq(reauth_time, "0"))
	{
		printf("no reauthentication");
	}
	else
	{
		printf("reauthentication every %ss", reauth_time);
	}
	if (!streq(version, "IKEv1"))
	{
		if (streq(rekey_time, "0"))
		{
			printf(", no rekeying");
		}
		else
		{
			printf(", rekeying every %ss", rekey_time);
		}
	}
	if (!streq(dpd_delay, "0"))
	{
		printf(", dpd delay %ss", dpd_delay);
	}
	printf("\n");

	ppk_id = vici_find_str(res, NULL, "%s.ppk_id", name);
	ppk_req = vici_find_str(res, NULL, "%s.ppk_required", name);
	if (ppk_id || ppk_req)
	{
		printf("  ppk: %s%s%srequired\n", ppk_id ?: "", ppk_id ? ", " : "",
			   !ppk_req || !streq(ppk_req, "yes") ? "not " : "");
	}

	ret = vici_parse_cb(res, conn_sn, NULL, conn_list, ike);
	free_hashtable(ike);
	return ret;
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

/*
 * Copyright (C) 2015 Tobias Brunner
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

#include "command.h"

#include <errno.h>

static int redirect(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *peer_ip = NULL, *peer_id = NULL, *ike = NULL, *gateway = NULL;
	int ret = 0, ike_id = 0;

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
			case 'i':
				ike = arg;
				continue;
			case 'I':
				ike_id = atoi(arg);
				continue;
			case 'p':
				peer_ip = arg;
				continue;
			case 'd':
				peer_id = arg;
				continue;
			case 'g':
				gateway = arg;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --redirect option");
		}
		break;
	}
	req = vici_begin("redirect");
	if (ike)
	{
		vici_add_key_valuef(req, "ike", "%s", ike);
	}
	if (ike_id)
	{
		vici_add_key_valuef(req, "ike-id", "%d", ike_id);
	}
	if (peer_ip)
	{
		vici_add_key_valuef(req, "peer-ip", "%s", peer_ip);
	}
	if (peer_id)
	{
		vici_add_key_valuef(req, "peer-id", "%s", peer_id);
	}
	if (gateway)
	{
		vici_add_key_valuef(req, "gateway", "%s", gateway);
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "redirect request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "redirect reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (streq(vici_find_str(res, "no", "success"), "yes"))
		{
			printf("redirect completed successfully\n");
		}
		else
		{
			fprintf(stderr, "redirect failed: %s\n",
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
		redirect, 'd', "redirect", "redirect an IKE_SA",
		{"--ike <name> | --ike-id <id> | --peer-ip <ip|subnet|range>",
		 "--peer-id <id|wildcards> | --gateway <ip|fqdn> [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"ike",			'i', 1, "redirect by IKE_SA name"},
			{"ike-id",		'I', 1, "redirect by IKE_SA unique identifier"},
			{"peer-ip",		'p', 1, "redirect by client IP"},
			{"peer-id",		'd', 1, "redirect by IKE_SA name"},
			{"gateway",		'g', 1, "target gateway (IP or FQDN)"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

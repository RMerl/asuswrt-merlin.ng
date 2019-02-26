/*
 * Copyright (C) 2017-2018 Tobias Brunner
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

static int rekey(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *child = NULL, *ike = NULL;
	int ret = 0, child_id = 0, ike_id = 0;
	bool reauth = FALSE;

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
			case 'i':
				ike = arg;
				continue;
			case 'C':
				child_id = atoi(arg);
				continue;
			case 'I':
				ike_id = atoi(arg);
				continue;
			case 'a':
				reauth = TRUE;
				continue;
			case EOF:
				break;
			default:
				return command_usage("invalid --rekey option");
		}
		break;
	}

	req = vici_begin("rekey");
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
	if (reauth)
	{
		vici_add_key_valuef(req, "reauth", "yes");
	}
	res = vici_submit(req, conn);
	if (!res)
	{
		ret = errno;
		fprintf(stderr, "rekey request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "rekey reply", format & COMMAND_FORMAT_PRETTY,
				  stdout);
	}
	else
	{
		if (streq(vici_find_str(res, "no", "success"), "yes"))
		{
			printf("rekey completed successfully\n");
		}
		else
		{
			fprintf(stderr, "rekey failed: %s\n",
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
		rekey, 'R', "rekey", "rekey an SA",
		{"--child <name> | --ike <name> | --child-id <id> | --ike-id <id>",
		 "[--reauth] [--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"child",		'c', 1, "rekey by CHILD_SA name"},
			{"ike",			'i', 1, "rekey by IKE_SA name"},
			{"child-id",	'C', 1, "rekey by CHILD_SA unique identifier"},
			{"ike-id",		'I', 1, "rekey by IKE_SA unique identifier"},
			{"reauth",		'a', 0, "reauthenticate instead of rekey an IKEv2 SA"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

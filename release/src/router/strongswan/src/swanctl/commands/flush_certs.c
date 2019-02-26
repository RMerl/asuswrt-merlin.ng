/*
 * Copyright (C) 2016 Andreas Steffen
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

#include <errno.h>

#include "command.h"

static int flush_certs(vici_conn_t *conn)
{
	vici_req_t *req;
	vici_res_t *res;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	char *arg, *type = NULL;
	int ret;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 't':
				type = arg;
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
				return command_usage("invalid --flush-certs option");
		}
		break;
	}
	req = vici_begin("flush-certs");

	if (type)
	{
		vici_add_key_valuef(req, "type", "%s", type);
	}
	res = vici_submit(req, conn);

	if (!res)
	{
		ret = errno;
		fprintf(stderr, "flush-certs request failed: %s\n", strerror(errno));
		return ret;
	}
	if (format & COMMAND_FORMAT_RAW)
	{
		vici_dump(res, "flush-certs reply", format & COMMAND_FORMAT_PRETTY,
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
		flush_certs, 'f', "flush-certs", "flush cached certificates",
		{"[--type x509|x509_ac|x509_crl|ocsp_response|pubkey]",
		 "[--raw|--pretty]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"type",		't', 1, "filter by certificate type"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

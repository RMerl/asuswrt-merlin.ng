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
#include <unistd.h>
#include <sys/stat.h>

#include "command.h"
#include "swanctl.h"
#include "load_creds.h"
#include "load_pools.h"
#include "load_conns.h"

static int load_all(vici_conn_t *conn)
{
	bool clear = FALSE, noprompt = FALSE;
	command_format_options_t format = COMMAND_FORMAT_NONE;
	settings_t *cfg;
	int ret = 0;
	char *arg;

	while (TRUE)
	{
		switch (command_getopt(&arg))
		{
			case 'h':
				return command_usage(NULL);
			case 'c':
				clear = TRUE;
				continue;
			case 'n':
				noprompt = TRUE;
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
				return command_usage("invalid --load-all option");
		}
		break;
	}

	cfg = settings_create(SWANCTL_CONF);
	if (!cfg)
	{
		fprintf(stderr, "parsing '%s' failed\n", SWANCTL_CONF);
		return EINVAL;
	}

	if (ret == 0)
	{
		ret = load_creds_cfg(conn, format, cfg, clear, noprompt);
	}
	if (ret == 0)
	{
		ret = load_pools_cfg(conn, format, cfg);
	}
	if (ret == 0)
	{
		ret = load_conns_cfg(conn, format, cfg);
	}

	cfg->destroy(cfg);

	return ret;
}

/**
 * Register the command.
 */
static void __attribute__ ((constructor))reg()
{
	command_register((command_t) {
		load_all, 'q', "load-all", "load credentials, pools and connections",
		{"[--raw|--pretty] [--clear] [--noprompt]"},
		{
			{"help",		'h', 0, "show usage information"},
			{"clear",		'c', 0, "clear previously loaded credentials"},
			{"noprompt",	'n', 0, "do not prompt for passwords"},
			{"raw",			'r', 0, "dump raw response message"},
			{"pretty",		'P', 0, "dump raw response message in pretty print"},
		}
	});
}

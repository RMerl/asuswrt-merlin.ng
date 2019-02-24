/*
 * Copyright (C) 2009 Martin Willi
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
#include "pki.h"

#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/optionsfrom.h>

/**
 * Registered commands.
 */
static command_t cmds[MAX_COMMANDS];

/**
 * active command.
 */
static int active = 0;

/**
 * number of registered commands
 */
static int registered = 0;

/**
 * help command index
 */
static int help_idx;

static int argc;

static char **argv;

static options_t *options;

/**
 * Global options used by all subcommands
 */
static struct option command_opts[MAX_COMMANDS > MAX_OPTIONS ? MAX_COMMANDS : MAX_OPTIONS];

/**
 * Global optstring used by all subcommands
 */
static char command_optstring[(MAX_COMMANDS > MAX_OPTIONS ? MAX_COMMANDS : MAX_OPTIONS) * 3];

/**
 * Build command_opts/command_optstr for the active command
 */
static void build_opts()
{
	int i, pos = 0;

	memset(command_opts, 0, sizeof(command_opts));
	memset(command_optstring, 0, sizeof(command_optstring));
	if (active == help_idx)
	{
		for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; i++)
		{
			command_opts[i].name = cmds[i].cmd;
			command_opts[i].val = cmds[i].op;
			command_optstring[i] = cmds[i].op;
		}
	}
	else
	{
		for (i = 0; cmds[active].options[i].name; i++)
		{
			command_opts[i].name = cmds[active].options[i].name;
			command_opts[i].has_arg = cmds[active].options[i].arg;
			command_opts[i].val = cmds[active].options[i].op;
			command_optstring[pos++] = cmds[active].options[i].op;
			switch (cmds[active].options[i].arg)
			{
				case optional_argument:
					command_optstring[pos++] = ':';
					/* FALL */
				case required_argument:
					command_optstring[pos++] = ':';
					/* FALL */
				case no_argument:
				default:
					break;
			}
		}
	}
}

/**
 * getopt_long wrapper
 */
int command_getopt(char **arg)
{
	int op;

	while (TRUE)
	{
		op = getopt_long(argc, argv, command_optstring, command_opts, NULL);
		switch (op)
		{
			case '+':
				if (!options->from(options, optarg, &argc, &argv, optind))
				{
					/* a error value */
					return 255;
				}
				continue;
			case 'v':
				dbg_default_set_level(atoi(optarg));
				continue;
			default:
				*arg = optarg;
				return op;
		}
	}
}

/**
 * Register a command
 */
void command_register(command_t command)
{
	int i;

	if (registered == MAX_COMMANDS)
	{
		fprintf(stderr, "unable to register command, please increase "
				"MAX_COMMANDS\n");
		return;
	}

	cmds[registered] = command;
	/* append default options, but not to --help */
	if (!active)
	{
		for (i = 0; i < countof(cmds[registered].options) - 1; i++)
		{
			if (!cmds[registered].options[i].name)
			{
				break;
			}
		}
		if (i > countof(cmds[registered].options) - 3)
		{
			fprintf(stderr, "command '%s' registered too many options, please "
					"increase MAX_OPTIONS\n", command.cmd);
		}
		else
		{
			cmds[registered].options[i++] = (command_option_t) {
				"debug",	'v', 1, "set debug level, default: 1"
			};
			cmds[registered].options[i++] = (command_option_t) {
				"options",	'+', 1, "read command line options from file"
			};
		}
		for (i = 0; cmds[registered].line[i]; i++)
		{
			if (i == MAX_LINES - 1)
			{
				fprintf(stderr, "command '%s' specifies too many usage summary "
						"lines, please increase MAX_LINES\n", command.cmd);
				break;
			}
		}
	}
	registered++;
}

/**
 * Print usage text, with an optional error
 */
int command_usage(char *error)
{
	FILE *out = stdout;
	int i, indent = 0;

	if (error)
	{
		out = stderr;
		fprintf(out, "Error: %s\n", error);
	}
	fprintf(out, "strongSwan %s PKI tool\n", VERSION);

	if (active == help_idx)
	{
		fprintf(out, "loaded plugins: %s\n",
				lib->plugins->loaded_plugins(lib->plugins));
	}

	fprintf(out, "usage:\n");
	if (active == help_idx)
	{
		for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; i++)
		{
			fprintf(out, "  pki --%-7s (-%c)  %s\n",
					cmds[i].cmd, cmds[i].op, cmds[i].description);
		}
	}
	else
	{
		for (i = 0; i < MAX_LINES && cmds[active].line[i]; i++)
		{
			if (i == 0)
			{
				indent = fprintf(out, "  pki --%s ", cmds[active].cmd);
				fprintf(out, "%s\n", cmds[active].line[i]);
			}
			else
			{
				fprintf(out, "%*s%s\n", indent, "", cmds[active].line[i]);
			}
		}
		for (i = 0; cmds[active].options[i].name; i++)
		{
			fprintf(out, "        --%-15s (-%c)  %s\n",
					cmds[active].options[i].name, cmds[active].options[i].op,
					cmds[active].options[i].desc);
		}
	}
	return error != NULL;
}


/**
 * Show usage information
 */
static int help(int argc, char *argv[])
{
	return command_usage(NULL);
}

/**
 * Dispatch cleanup hook
 */
static void cleanup()
{
	options->destroy(options);
}

/**
 * Dispatch commands.
 */
int command_dispatch(int c, char *v[])
{
	int op, i;

	options = options_create();
	atexit(cleanup);
	active = help_idx = registered;
	argc = c;
	argv = v;
	command_register((command_t){help, 'h', "help", "show usage information"});

	build_opts();
	op = getopt_long(c, v, command_optstring, command_opts, NULL);
	for (i = 0; i < MAX_COMMANDS && cmds[i].cmd; i++)
	{
		if (cmds[i].op == op)
		{
			active = i;
			build_opts();
			return cmds[i].call();
		}
	}
	return command_usage(c > 1 ? "invalid operation" : NULL);
}

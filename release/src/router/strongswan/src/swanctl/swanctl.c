/*
 * Copyright (C) 2018 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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

#include "swanctl.h"
#include "command.h"

#include <unistd.h>

#include <library.h>

/*
 * Described in header
 */
char *swanctl_dir;

/*
 * Described in header
 */
settings_t *load_swanctl_conf(char *file)
{
	settings_t *cfg;
	char buf[PATH_MAX];

	if (!file)
	{
		if (!strlen(swanctl_dir))
		{
			free(swanctl_dir);
			swanctl_dir = strdup(getcwd(buf, sizeof(buf)));
		}
		file = buf;
		snprintf(buf, sizeof(buf), "%s%s%s", swanctl_dir,
				 DIRECTORY_SEPARATOR, SWANCTL_CONF);
	}

	cfg = settings_create(file);
	if (!cfg)
	{
		fprintf(stderr, "parsing '%s' failed\n", file);
		return NULL;
	}
	free(swanctl_dir);
	swanctl_dir = path_dirname(file);
	return cfg;
}

/**
 * Cleanup library atexit()
 */
static void cleanup()
{
	free(swanctl_dir);
	lib->processor->cancel(lib->processor);
	library_deinit();
}

/**
 * Library initialization and operation parsing
 */
int main(int argc, char *argv[])
{
	atexit(cleanup);
	if (!library_init(NULL, "swanctl"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "swanctl", argv[0]))
	{
		fprintf(stderr, "integrity check of swanctl failed\n");
		exit(SS_RC_DAEMON_INTEGRITY);
	}
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "swanctl.load", PLUGINS)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	swanctl_dir = strdup(getenv("SWANCTL_DIR") ?: SWANCTLDIR);

	dbg_default_set_level(0);
	lib->processor->set_threads(lib->processor, 4);
	dbg_default_set_level(1);

	return command_dispatch(argc, argv);
}

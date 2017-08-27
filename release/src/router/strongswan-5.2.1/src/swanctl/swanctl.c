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

#include "command.h"

#include <unistd.h>

#include <library.h>

/**
 * Cleanup library atexit()
 */
static void cleanup()
{
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
	dbg_default_set_level(0);
	lib->processor->set_threads(lib->processor, 4);
	dbg_default_set_level(1);

	return command_dispatch(argc, argv);
}

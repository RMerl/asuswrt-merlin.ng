/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <fast_dispatcher.h>
#include <utils/debug.h>
#include <stdio.h>

#include "manager.h"
#include "storage.h"
#include "controller/auth_controller.h"
#include "controller/ikesa_controller.h"
#include "controller/gateway_controller.h"
#include "controller/control_controller.h"
#include "controller/config_controller.h"

int main (int arc, char *argv[])
{
	fast_dispatcher_t *dispatcher;
	storage_t *storage;
	char *socket;
	char *database;
	bool debug;
	int threads, timeout;

	library_init(NULL, "manager");
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "manager.load", PLUGINS)))
	{
		return 1;
	}

	socket = lib->settings->get_str(lib->settings, "manager.socket", NULL);
	debug = lib->settings->get_bool(lib->settings, "manager.debug", FALSE);
	timeout = lib->settings->get_time(lib->settings, "manager.timeout", 900);
	threads = lib->settings->get_int(lib->settings, "manager.threads", 10);
	database = lib->settings->get_str(lib->settings, "manager.database", NULL);
	if (!database)
	{
		DBG1(DBG_LIB, "database URI undefined, set manager.database "
			 "in strongswan.conf");
		//return 1;
	}

	storage = storage_create(database);
	if (storage == NULL)
	{
		return 1;
	}

	dispatcher = fast_dispatcher_create(socket, debug, timeout,
						(fast_context_constructor_t)manager_create, storage);
	dispatcher->add_controller(dispatcher, ikesa_controller_create, NULL);
	dispatcher->add_controller(dispatcher, gateway_controller_create, NULL);
	dispatcher->add_controller(dispatcher, auth_controller_create, NULL);
	dispatcher->add_controller(dispatcher, control_controller_create, NULL);
	dispatcher->add_controller(dispatcher, config_controller_create, NULL);

	dispatcher->run(dispatcher, threads);

	dispatcher->waitsignal(dispatcher);

	dispatcher->destroy(dispatcher);
	storage->destroy(storage);

	library_deinit();

	return 0;
}

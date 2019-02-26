/*
 * Copyright (C) 2010 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "socket_dynamic_plugin.h"

#include "socket_dynamic_socket.h"

#include <daemon.h>

typedef struct private_socket_dynamic_plugin_t private_socket_dynamic_plugin_t;

/**
 * Private data of socket plugin
 */
struct private_socket_dynamic_plugin_t {

	/**
	 * Implements plugin interface
	 */
	socket_dynamic_plugin_t public;
};

METHOD(plugin_t, get_features, int,
	private_socket_dynamic_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(socket_register, socket_dynamic_socket_create),
			PLUGIN_PROVIDE(CUSTOM, "socket"),
				PLUGIN_SDEPEND(CUSTOM, "kernel-ipsec"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, get_name, char*,
	private_socket_dynamic_plugin_t *this)
{
	return "socket-dynamic";
}

METHOD(plugin_t, destroy, void,
	private_socket_dynamic_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *socket_dynamic_plugin_create()
{
	private_socket_dynamic_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}


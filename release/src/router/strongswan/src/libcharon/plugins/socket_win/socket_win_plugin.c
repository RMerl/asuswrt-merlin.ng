/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "socket_win_plugin.h"
#include "socket_win_socket.h"

#include <daemon.h>

typedef struct private_socket_win_plugin_t private_socket_win_plugin_t;

/**
 * Private data of socket plugin
 */
struct private_socket_win_plugin_t {

	/**
	 * Implements plugin interface
	 */
	socket_win_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_socket_win_plugin_t *this)
{
	return "socket-win";
}

METHOD(plugin_t, destroy, void,
	private_socket_win_plugin_t *this)
{
	free(this);
}

METHOD(plugin_t, get_features, int,
	private_socket_win_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(socket_register, socket_win_socket_create),
			PLUGIN_PROVIDE(CUSTOM, "socket"),
				PLUGIN_DEPENDS(CUSTOM, "kernel-ipsec"),
	};
	*features = f;
	return countof(f);
}

/**
 * Create instance of socket-win plugin
 */
plugin_t *socket_win_plugin_create()
{
	private_socket_win_plugin_t *this;

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

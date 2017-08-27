/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "lookip_plugin.h"

#include "lookip_listener.h"
#include "lookip_socket.h"

#include <daemon.h>

typedef struct private_lookip_plugin_t private_lookip_plugin_t;

/**
 * private data of lookip plugin
 */
struct private_lookip_plugin_t {

	/**
	 * implements plugin interface
	 */
	lookip_plugin_t public;

	/**
	 * Listener collecting virtual IP assignements
	 */
	lookip_listener_t *listener;

	/**
	 * UNIX socket to serve client queries
	 */
	lookip_socket_t *socket;
};

METHOD(plugin_t, get_name, char*,
	private_lookip_plugin_t *this)
{
	return "lookip";
}

/**
 * Register listener
 */
static bool plugin_cb(private_lookip_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_lookip_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "lookip"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_lookip_plugin_t *this)
{
	DESTROY_IF(this->socket);
	this->listener->destroy(this->listener);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *lookip_plugin_create()
{
	private_lookip_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.listener = lookip_listener_create(),
	);

	this->socket = lookip_socket_create(this->listener);
	if (!this->socket)
	{
		destroy(this);
		return NULL;
	}

	return &this->public.plugin;
}

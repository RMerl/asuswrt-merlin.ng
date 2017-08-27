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

#include "error_notify_plugin.h"

#include "error_notify_listener.h"
#include "error_notify_socket.h"

#include <daemon.h>

typedef struct private_error_notify_plugin_t private_error_notify_plugin_t;

/**
 * private data of error_notify plugin
 */
struct private_error_notify_plugin_t {

	/**
	 * Implements plugin interface
	 */
	error_notify_plugin_t public;

	/**
	 * Listener catching error alerts
	 */
	error_notify_listener_t *listener;

	/**
	 * Socket sending notifications
	 */
	error_notify_socket_t *socket;
};

METHOD(plugin_t, get_name, char*,
	private_error_notify_plugin_t *this)
{
	return "error-notify";
}

/**
 * Register listener
 */
static bool plugin_cb(private_error_notify_plugin_t *this,
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
	private_error_notify_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "error-notify"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_error_notify_plugin_t *this)
{
	this->listener->destroy(this->listener);
	this->socket->destroy(this->socket);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *error_notify_plugin_create()
{
	private_error_notify_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.socket = error_notify_socket_create(),
	);

	if (!this->socket)
	{
		free(this);
		return NULL;
	}

	this->listener = error_notify_listener_create(this->socket);

	return &this->public.plugin;
}

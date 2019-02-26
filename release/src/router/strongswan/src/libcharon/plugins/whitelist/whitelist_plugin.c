/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "whitelist_plugin.h"

#include "whitelist_listener.h"
#include "whitelist_control.h"

#include <daemon.h>

typedef struct private_whitelist_plugin_t private_whitelist_plugin_t;

/**
 * private data of whitelist plugin
 */
struct private_whitelist_plugin_t {

	/**
	 * implements plugin interface
	 */
	whitelist_plugin_t public;

	/**
	 * Listener checking whitelist entries during authorization
	 */
	whitelist_listener_t *listener;

	/**
	 * Whitelist control socket
	 */
	whitelist_control_t *control;
};

METHOD(plugin_t, get_name, char*,
	private_whitelist_plugin_t *this)
{
	return "whitelist";
}

/**
 * Register listener
 */
static bool plugin_cb(private_whitelist_plugin_t *this,
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
	private_whitelist_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "whitelist"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_whitelist_plugin_t *this)
{
	this->listener->destroy(this->listener);
	DESTROY_IF(this->control);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *whitelist_plugin_create()
{
	private_whitelist_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.listener = whitelist_listener_create(),
	);

	this->control = whitelist_control_create(this->listener);
	if (!this->control)
	{
		destroy(this);
		return NULL;
	}

	return &this->public.plugin;
}

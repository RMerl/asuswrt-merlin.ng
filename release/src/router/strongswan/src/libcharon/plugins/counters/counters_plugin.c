/*
 * Copyright (C) 2017 Tobias Brunner
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

#include "counters_plugin.h"

#include <daemon.h>
#include <plugins/plugin_feature.h>

#include "counters_listener.h"

typedef struct private_counters_plugin_t private_counters_plugin_t;

/**
 * Private data
 */
struct private_counters_plugin_t {

	/**
	 * Public interface
	 */
	counters_plugin_t public;

	/**
	 * Listener implementation
	 */
	counters_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_counters_plugin_t *this)
{
	return "counters";
}

/**
 * Register listener
 */
static bool plugin_cb(private_counters_plugin_t *this,
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
	private_counters_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "counters"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_counters_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/*
 * Described in header
 */
plugin_t *counters_plugin_create()
{
	private_counters_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.listener = counters_listener_create(),
	);

	return &this->public.plugin;
}

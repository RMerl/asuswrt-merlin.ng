/*
 * Copyright (C) 2010-2014 Martin Willi
 * Copyright (C) 2010-2014 revosec AG
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

#include "forecast_plugin.h"
#include "forecast_listener.h"
#include "forecast_forwarder.h"

#include <daemon.h>

typedef struct private_forecast_plugin_t private_forecast_plugin_t;

/**
 * Private data of forecast plugin
 */
struct private_forecast_plugin_t {

	/**
	 * implements plugin interface
	 */
	forecast_plugin_t public;

	/**
	 * Listener registering active tunnels
	 */
	forecast_listener_t *listener;

	/**
	 * Broadcast/Multicast sniffer and forwarder
	 */
	forecast_forwarder_t *forwarder;
};

METHOD(plugin_t, get_name, char*,
	private_forecast_plugin_t *this)
{
	return "forecast";
}

/**
 * Register plugin features
 */
static bool register_forecast(private_forecast_plugin_t *this,
							  plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		this->forwarder = forecast_forwarder_create(this->listener);
		if (!this->forwarder)
		{
			return FALSE;
		}
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
		this->forwarder->destroy(this->forwarder);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_forecast_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)register_forecast, NULL),
			PLUGIN_PROVIDE(CUSTOM, "forecast"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_forecast_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *forecast_plugin_create()
{
	private_forecast_plugin_t *this;

	if (!lib->caps->keep(lib->caps, CAP_NET_RAW))
	{
		DBG1(DBG_NET, "forecast plugin requires CAP_NET_RAW capability");
		return NULL;
	}

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = (void*)return_false,
				.destroy = _destroy,
			},
		},
		.listener = forecast_listener_create(),
	);

	return &this->public.plugin;
}

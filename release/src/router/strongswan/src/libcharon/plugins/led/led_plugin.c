/*
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

#include "led_plugin.h"

#include "led_listener.h"

#include <daemon.h>

typedef struct private_led_plugin_t private_led_plugin_t;

/**
 * private data of led plugin
 */
struct private_led_plugin_t {

	/**
	 * implements plugin interface
	 */
	led_plugin_t public;

	/**
	 * Listener controlling LEDs
	 */
	led_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_led_plugin_t *this)
{
	return "led";
}

/**
 * Register listener
 */
static bool plugin_cb(private_led_plugin_t *this,
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
	private_led_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "led"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_led_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *led_plugin_create()
{
	private_led_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.listener = led_listener_create(),
	);

	return &this->public.plugin;
}

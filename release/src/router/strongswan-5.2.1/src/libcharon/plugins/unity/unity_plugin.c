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

#include "unity_plugin.h"
#include "unity_handler.h"
#include "unity_narrow.h"
#include "unity_provider.h"

#include <daemon.h>
#include <hydra.h>

typedef struct private_unity_plugin_t private_unity_plugin_t;

/**
 * private data of unity_plugin
 */
struct private_unity_plugin_t {

	/**
	 * public functions
	 */
	unity_plugin_t public;

	/**
	 * Handler for UNITY configuration attributes
	 */
	unity_handler_t *handler;

	/**
	 * Responder Unity configuration attribute provider
	 */
	unity_provider_t *provider;

	/**
	 * Traffic selector narrower, for Unity Split-Includes
	 */
	unity_narrow_t *narrower;
};

METHOD(plugin_t, get_name, char*,
	private_unity_plugin_t *this)
{
	return "unity";
}

/**
 * Register listener
 */
static bool plugin_cb(private_unity_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		hydra->attributes->add_handler(hydra->attributes,
									   &this->handler->handler);
		hydra->attributes->add_provider(hydra->attributes,
										&this->provider->provider);
		charon->bus->add_listener(charon->bus, &this->narrower->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->narrower->listener);
		hydra->attributes->remove_handler(hydra->attributes,
										  &this->handler->handler);
		hydra->attributes->remove_provider(hydra->attributes,
										   &this->provider->provider);

	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_unity_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "unity"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_unity_plugin_t *this)
{
	this->narrower->destroy(this->narrower);
	this->handler->destroy(this->handler);
	this->provider->destroy(this->provider);
	free(this);
}

/*
 * see header file
 */
plugin_t *unity_plugin_create()
{
	private_unity_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.handler = unity_handler_create(),
		.provider = unity_provider_create(),
	);
	this->narrower = unity_narrow_create(this->handler);

	return &this->public.plugin;
}

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

#include "certexpire_plugin.h"

#include "certexpire_listener.h"
#include "certexpire_export.h"

#include <daemon.h>

typedef struct private_certexpire_plugin_t private_certexpire_plugin_t;

/**
 * Private data of certexpire plugin
 */
struct private_certexpire_plugin_t {

	/**
	 * Implements plugin interface
	 */
	certexpire_plugin_t public;

	/**
	 * Listener collecting expire information
	 */
	certexpire_listener_t *listener;

	/**
	 * Cache and export trustchain expire information
	 */
	certexpire_export_t *export;
};

METHOD(plugin_t, get_name, char*,
	private_certexpire_plugin_t *this)
{
	return "certexpire";
}

/**
 * Register listener
 */
static bool plugin_cb(private_certexpire_plugin_t *this,
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
	private_certexpire_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "certexpire"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_certexpire_plugin_t *this)
{
	this->listener->destroy(this->listener);
	this->export->destroy(this->export);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *certexpire_plugin_create()
{
	private_certexpire_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.export = certexpire_export_create(),
	);
	this->listener = certexpire_listener_create(this->export);

	return &this->public.plugin;
}

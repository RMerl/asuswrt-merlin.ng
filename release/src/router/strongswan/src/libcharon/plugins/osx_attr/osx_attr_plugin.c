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

#include "osx_attr_plugin.h"
#include "osx_attr_handler.h"

#include <daemon.h>

typedef struct private_osx_attr_plugin_t private_osx_attr_plugin_t;

/**
 * Private data of an osx_attr_plugin_t object.
 */
struct private_osx_attr_plugin_t {

	/**
	 * Public interface
	 */
	osx_attr_plugin_t public;

	/**
	 * Android specific DNS handler
	 */
	osx_attr_handler_t *handler;
};

METHOD(plugin_t, get_name, char*,
	private_osx_attr_plugin_t *this)
{
	return "osx-attr";
}

/**
 * Register handler
 */
static bool plugin_cb(private_osx_attr_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		charon->attributes->add_handler(charon->attributes,
										&this->handler->handler);
	}
	else
	{
		charon->attributes->remove_handler(charon->attributes,
										   &this->handler->handler);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_osx_attr_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "osx-attr"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_osx_attr_plugin_t *this)
{
	this->handler->destroy(this->handler);
	free(this);
}

/**
 * See header
 */
plugin_t *osx_attr_plugin_create()
{
	private_osx_attr_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.handler = osx_attr_handler_create(),
	);

	return &this->public.plugin;
}

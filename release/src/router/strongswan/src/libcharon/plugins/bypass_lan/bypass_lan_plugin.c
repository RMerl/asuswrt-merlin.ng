/*
 * Copyright (C) 2016 Tobias Brunner
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

#include "bypass_lan_plugin.h"
#include "bypass_lan_listener.h"

#include <daemon.h>

typedef struct private_bypass_lan_plugin_t private_bypass_lan_plugin_t;

/**
 * Private data
 */
struct private_bypass_lan_plugin_t {

	/**
	 * Public interface
	 */
	bypass_lan_plugin_t public;

	/**
	 * Listener installing bypass policies
	 */
	bypass_lan_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_bypass_lan_plugin_t *this)
{
	return "bypass-lan";
}

/**
 * Register listener
 */
static bool plugin_cb(private_bypass_lan_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		charon->kernel->add_listener(charon->kernel,
									 &this->listener->listener);
	}
	else
	{
		charon->kernel->remove_listener(charon->kernel,
										&this->listener->listener);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_bypass_lan_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "bypass-lan"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, reload, bool,
	private_bypass_lan_plugin_t *this)
{
	this->listener->reload_interfaces(this->listener);
	return TRUE;
}

METHOD(plugin_t, destroy, void,
	private_bypass_lan_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *bypass_lan_plugin_create()
{
	private_bypass_lan_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
		.listener = bypass_lan_listener_create(),
	);

	return &this->public.plugin;
}

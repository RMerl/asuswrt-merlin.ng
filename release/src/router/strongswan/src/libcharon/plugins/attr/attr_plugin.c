/*
 * Copyright (C) 2009 Martin Willi
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

#include "attr_plugin.h"
#include "attr_provider.h"

#include <daemon.h>

typedef struct private_attr_plugin_t private_attr_plugin_t;

/**
 * private data of attr plugin
 */
struct private_attr_plugin_t {

	/**
	 * implements plugin interface
	 */
	attr_plugin_t public;

	/**
	 * CFG attributes provider
	 */
	attr_provider_t *provider;
};

METHOD(plugin_t, get_name, char*,
	private_attr_plugin_t *this)
{
	return "attr";
}

/**
 * Register provider
 */
static bool plugin_cb(private_attr_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		charon->attributes->add_provider(charon->attributes,
										 &this->provider->provider);
	}
	else
	{
		charon->attributes->remove_provider(charon->attributes,
										    &this->provider->provider);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_attr_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "attr"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, reload, bool,
	private_attr_plugin_t *this)
{
	this->provider->reload(this->provider);
	return TRUE;
}

METHOD(plugin_t, destroy, void,
	private_attr_plugin_t *this)
{
	this->provider->destroy(this->provider);
	free(this);
}

/*
 * see header file
 */
plugin_t *attr_plugin_create()
{
	private_attr_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
		.provider = attr_provider_create(),
	);

	return &this->public.plugin;
}

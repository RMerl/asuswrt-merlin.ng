/*
 * Copyright (C) 2010 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include "maemo_plugin.h"
#include "maemo_service.h"

#include <daemon.h>

typedef struct private_maemo_plugin_t private_maemo_plugin_t;

/**
 * private data of maemo plugin
 */
struct private_maemo_plugin_t {

	/**
	 * implements plugin interface
	 */
	maemo_plugin_t public;

	/**
	 * service
	 */
	maemo_service_t *service;
};

METHOD(plugin_t, get_name, char*,
	private_maemo_plugin_t *this)
{
	return "maemo";
}

METHOD(plugin_t, get_features, int,
	private_maemo_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_NOOP,
			PLUGIN_PROVIDE(CUSTOM, "maemo"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_maemo_plugin_t *this)
{
	this->service->destroy(this->service);
	free(this);
}

/*
 * See header
 */
plugin_t *maemo_plugin_create()
{
	private_maemo_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	this->service = maemo_service_create();
	if (!this->service)
	{
		return NULL;
	}

	return &this->public.plugin;
}

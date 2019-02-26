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

#include "p_cscf_plugin.h"
#include "p_cscf_handler.h"

#include <daemon.h>

typedef struct private_p_cscf_plugin_t private_p_cscf_plugin_t;

/**
 * Private data
 */
struct private_p_cscf_plugin_t {

	/**
	 * Public interface
	 */
	p_cscf_plugin_t public;

	/**
	 * P-CSCF server address attribute handler
	 */
	p_cscf_handler_t *handler;
};

METHOD(plugin_t, get_name, char*,
	private_p_cscf_plugin_t *this)
{
	return "p-cscf";
}

/**
 * Register handler
 */
static bool plugin_cb(private_p_cscf_plugin_t *this,
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
	private_p_cscf_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "p-cscf"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_p_cscf_plugin_t *this)
{
	this->handler->destroy(this->handler);
	free(this);
}

/**
 * See header
 */
plugin_t *p_cscf_plugin_create()
{
	private_p_cscf_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.handler = p_cscf_handler_create(),
	);

	return &this->public.plugin;
}

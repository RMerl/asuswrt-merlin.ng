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

#include "pkcs7_plugin.h"
#include "pkcs7_generic.h"
#include "pkcs7_data.h"
#include "pkcs7_signed_data.h"
#include "pkcs7_enveloped_data.h"

#include <library.h>

typedef struct private_pkcs7_plugin_t private_pkcs7_plugin_t;

/**
 * private data of pkcs7_plugin
 */
struct private_pkcs7_plugin_t {

	/**
	 * public functions
	 */
	pkcs7_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pkcs7_plugin_t *this)
{
	return "pkcs7";
}

METHOD(plugin_t, get_features, int,
	private_pkcs7_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CONTAINER_DECODE, pkcs7_generic_load, TRUE),
			PLUGIN_PROVIDE(CONTAINER_DECODE, CONTAINER_PKCS7),
		PLUGIN_REGISTER(CONTAINER_ENCODE, pkcs7_data_gen, TRUE),
			PLUGIN_PROVIDE(CONTAINER_ENCODE, CONTAINER_PKCS7_DATA),
		PLUGIN_REGISTER(CONTAINER_ENCODE, pkcs7_signed_data_gen, TRUE),
			PLUGIN_PROVIDE(CONTAINER_ENCODE, CONTAINER_PKCS7_SIGNED_DATA),
		PLUGIN_REGISTER(CONTAINER_ENCODE, pkcs7_enveloped_data_gen, TRUE),
			PLUGIN_PROVIDE(CONTAINER_ENCODE, CONTAINER_PKCS7_ENVELOPED_DATA),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_pkcs7_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *pkcs7_plugin_create()
{
	private_pkcs7_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}

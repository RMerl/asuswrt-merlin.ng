/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "pkcs8_plugin.h"

#include <library.h>

#include "pkcs8_builder.h"

typedef struct private_pkcs8_plugin_t private_pkcs8_plugin_t;

/**
 * private data of pkcs8_plugin
 */
struct private_pkcs8_plugin_t {

	/**
	 * public functions
	 */
	pkcs8_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pkcs8_plugin_t *this)
{
	return "pkcs8";
}

METHOD(plugin_t, get_features, int,
	private_pkcs8_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRIVKEY, pkcs8_private_key_load, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED25519),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED448),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_pkcs8_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *pkcs8_plugin_create()
{
	private_pkcs8_plugin_t *this;

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


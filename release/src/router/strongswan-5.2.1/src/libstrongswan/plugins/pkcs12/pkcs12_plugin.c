/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "pkcs12_plugin.h"

#include <library.h>

#include "pkcs12_decode.h"

typedef struct private_pkcs12_plugin_t private_pkcs12_plugin_t;

/**
 * private data of pkcs12_plugin
 */
struct private_pkcs12_plugin_t {

	/**
	 * public functions
	 */
	pkcs12_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pkcs12_plugin_t *this)
{
	return "pkcs12";
}

METHOD(plugin_t, get_features, int,
	private_pkcs12_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CONTAINER_DECODE, pkcs12_decode, FALSE),
			PLUGIN_PROVIDE(CONTAINER_DECODE, CONTAINER_PKCS12),
				PLUGIN_DEPENDS(CONTAINER_DECODE, CONTAINER_PKCS7),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ANY),
				PLUGIN_SDEPEND(HASHER, HASH_SHA1),
				PLUGIN_SDEPEND(CRYPTER, ENCR_3DES, 24),
				PLUGIN_SDEPEND(CRYPTER, ENCR_RC2_CBC, 0),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_pkcs12_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *pkcs12_plugin_create()
{
	private_pkcs12_plugin_t *this;

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


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

#include "pkcs1_plugin.h"

#include <library.h>
#include "pkcs1_builder.h"
#include "pkcs1_encoder.h"

typedef struct private_pkcs1_plugin_t private_pkcs1_plugin_t;

/**
 * private data of pkcs1_plugin
 */
struct private_pkcs1_plugin_t {

	/**
	 * public functions
	 */
	pkcs1_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pkcs1_plugin_t *this)
{
	return "pkcs1";
}

METHOD(plugin_t, get_features, int,
	private_pkcs1_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRIVKEY, pkcs1_private_key_load, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
				PLUGIN_SDEPEND(PRIVKEY, KEY_RSA),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ECDSA),
		PLUGIN_REGISTER(PRIVKEY, pkcs1_private_key_load, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
		PLUGIN_REGISTER(PUBKEY, pkcs1_public_key_load, FALSE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ANY),
				PLUGIN_SDEPEND(PUBKEY, KEY_RSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_ECDSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_ED25519),
				PLUGIN_SDEPEND(PUBKEY, KEY_ED448),
				PLUGIN_SDEPEND(PUBKEY, KEY_BLISS),
				PLUGIN_SDEPEND(PUBKEY, KEY_DSA),
		PLUGIN_REGISTER(PUBKEY, pkcs1_public_key_load, FALSE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_pkcs1_plugin_t *this)
{
	lib->encoding->remove_encoder(lib->encoding, pkcs1_encoder_encode);

	free(this);
}

/*
 * see header file
 */
plugin_t *pkcs1_plugin_create()
{
	private_pkcs1_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	lib->encoding->add_encoder(lib->encoding, pkcs1_encoder_encode);

	return &this->public.plugin;
}


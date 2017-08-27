/*
 * Copyright (C) 2008 Martin Willi
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

#include "pubkey_plugin.h"

#include <library.h>
#include "pubkey_cert.h"

typedef struct private_pubkey_plugin_t private_pubkey_plugin_t;

/**
 * private data of pubkey_plugin
 */
struct private_pubkey_plugin_t {

	/**
	 * public functions
	 */
	pubkey_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pubkey_plugin_t *this)
{
	return "pubkey";
}

METHOD(plugin_t, get_features, int,
	private_pubkey_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CERT_ENCODE, pubkey_cert_wrap, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_TRUSTED_PUBKEY),
		PLUGIN_REGISTER(CERT_DECODE, pubkey_cert_wrap, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_TRUSTED_PUBKEY),
				PLUGIN_SDEPEND(PUBKEY, KEY_RSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_ECDSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_DSA),
	};
	*features = f;
	return countof(f);
}
METHOD(plugin_t, destroy, void,
	private_pubkey_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *pubkey_plugin_create()
{
	private_pubkey_plugin_t *this;

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


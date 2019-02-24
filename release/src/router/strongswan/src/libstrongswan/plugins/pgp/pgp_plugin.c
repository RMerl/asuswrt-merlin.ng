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

#include "pgp_plugin.h"

#include <library.h>
#include "pgp_builder.h"
#include "pgp_encoder.h"
#include "pgp_cert.h"

typedef struct private_pgp_plugin_t private_pgp_plugin_t;

/**
 * private data of pgp_plugin
 */
struct private_pgp_plugin_t {

	/**
	 * public functions
	 */
	pgp_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_pgp_plugin_t *this)
{
	return "pgp";
}

METHOD(plugin_t, get_features, int,
	private_pgp_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRIVKEY, pgp_private_key_load, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PRIVKEY, pgp_private_key_load, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),

		PLUGIN_REGISTER(PUBKEY, pgp_public_key_load, FALSE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ANY),
		PLUGIN_REGISTER(PUBKEY, pgp_public_key_load, FALSE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),

		PLUGIN_REGISTER(CERT_DECODE, pgp_cert_load, FALSE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_GPG),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_pgp_plugin_t *this)
{
	lib->encoding->remove_encoder(lib->encoding, pgp_encoder_encode);

	free(this);
}

/*
 * see header file
 */
plugin_t *pgp_plugin_create()
{
	private_pgp_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	lib->encoding->add_encoder(lib->encoding, pgp_encoder_encode);

	return &this->public.plugin;
}


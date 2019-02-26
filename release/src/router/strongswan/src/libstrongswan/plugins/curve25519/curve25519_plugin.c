/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * Copyright (C) 2016 Andreas Steffen
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

#include "curve25519_plugin.h"
#include "curve25519_dh.h"
#include "curve25519_private_key.h"
#include "curve25519_public_key.h"
#include "curve25519_identity_hasher.h"

#include <library.h>

typedef struct private_curve25519_plugin_t private_curve25519_plugin_t;

/**
 * private data of curve25519_plugin
 */
struct private_curve25519_plugin_t {

	/**
	 * public functions
	 */
	curve25519_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_curve25519_plugin_t *this)
{
	return "curve25519";
}

METHOD(plugin_t, get_features, int,
	private_curve25519_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* X25519 DH group */
		PLUGIN_REGISTER(DH, curve25519_dh_create),
			PLUGIN_PROVIDE(DH, CURVE_25519),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
		/* Ed25519 private/public keys */
		PLUGIN_REGISTER(PRIVKEY, curve25519_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ED25519),
		PLUGIN_REGISTER(PRIVKEY_GEN, curve25519_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ED25519),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
		PLUGIN_REGISTER(PUBKEY, curve25519_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ED25519),
		/* Ed25519 signature scheme, private */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ED25519),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
		/* Ed25519 signature verification scheme, public */
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ED25519),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
		/* register a pro forma identity hasher */
		PLUGIN_REGISTER(HASHER, curve25519_identity_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_IDENTITY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_curve25519_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *curve25519_plugin_create()
{
	private_curve25519_plugin_t *this;

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

/*
 * Copyright (C) 2014-2016 Andreas Steffen
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

#include "bliss_plugin.h"
#include "bliss_private_key.h"
#include "bliss_public_key.h"

#include <library.h>

typedef struct private_bliss_plugin_t private_bliss_plugin_t;

/**
 * private data of bliss_plugin
 */
struct private_bliss_plugin_t {

	/**
	 * public functions
	 */
	bliss_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_bliss_plugin_t *this)
{
	return "bliss";
}

METHOD(plugin_t, get_features, int,
	private_bliss_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* private/public keys */
		PLUGIN_REGISTER(PRIVKEY, bliss_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_BLISS),
		PLUGIN_REGISTER(PRIVKEY, bliss_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PRIVKEY_GEN, bliss_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_BLISS),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_SDEPEND(XOF, XOF_MGF1_SHA1),
				PLUGIN_SDEPEND(XOF, XOF_MGF1_SHA256),
		PLUGIN_REGISTER(PUBKEY, bliss_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_BLISS),
		PLUGIN_REGISTER(PUBKEY, bliss_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ANY),
		/* signature schemes, private */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA2_256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA256),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA2_384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA384),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA2_512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA3_256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_256),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA3_384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_384),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_BLISS_WITH_SHA3_512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_512),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		/* signature verification schemes */
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA2_256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA256),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA2_384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA384),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA2_512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA3_256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_256),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA3_384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_384),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_BLISS_WITH_SHA3_512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA3_512),
			PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA512),
	};
	*features = f;

	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_bliss_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *bliss_plugin_create()
{
	private_bliss_plugin_t *this;

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

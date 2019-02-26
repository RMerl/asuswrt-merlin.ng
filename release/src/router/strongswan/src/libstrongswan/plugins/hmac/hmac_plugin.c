/*
 * Copyright (C) 2008 Martin Willi
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

#include "hmac_plugin.h"

#include <library.h>
#include "hmac.h"

typedef struct private_hmac_plugin_t private_hmac_plugin_t;

/**
 * private data of hmac_plugin
 */
struct private_hmac_plugin_t {

	/**
	 * public functions
	 */
	hmac_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_hmac_plugin_t *this)
{
	return "hmac";
}

METHOD(plugin_t, get_features, int,
	private_hmac_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRF, hmac_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA1),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA1),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_MD5),
				PLUGIN_DEPENDS(HASHER,  HASH_MD5),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_256),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA256),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_384),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA384),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_512),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA512),
		PLUGIN_REGISTER(SIGNER, hmac_signer_create),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_96),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA1),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_128),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA1),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_160),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA1),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_96),
				PLUGIN_DEPENDS(HASHER,  HASH_MD5),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_128),
				PLUGIN_DEPENDS(HASHER,  HASH_MD5),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_128),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA256),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_256),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA256),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_192),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA384),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_384),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA384),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_256),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA512),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_512),
				PLUGIN_DEPENDS(HASHER,  HASH_SHA512),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_hmac_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *hmac_plugin_create()
{
	private_hmac_plugin_t *this;

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


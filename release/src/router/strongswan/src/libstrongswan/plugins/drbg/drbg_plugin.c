/*
 * Copyright (C) 2019 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "drbg_plugin.h"
#include "drbg_ctr.h"
#include "drbg_hmac.h"

#include <library.h>

typedef struct private_drbg_plugin_t private_drbg_plugin_t;

/**
 * private data of drbg_plugin
 */
struct private_drbg_plugin_t {

	/**
	 * public functions
	 */
	drbg_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_drbg_plugin_t *this)
{
	return "drbg";
}

METHOD(plugin_t, get_features, int,
	private_drbg_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* NIST CTR DRBG */
		PLUGIN_REGISTER(DRBG, drbg_ctr_create),
			PLUGIN_PROVIDE(DRBG, DRBG_CTR_AES128),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_ECB, 16),
			PLUGIN_PROVIDE(DRBG, DRBG_CTR_AES192),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_ECB, 24),
			PLUGIN_PROVIDE(DRBG, DRBG_CTR_AES256),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_ECB, 32),
		/* NIST HMAC DRBG */
		PLUGIN_REGISTER(DRBG, drbg_hmac_create),
			PLUGIN_PROVIDE(DRBG, DRBG_HMAC_SHA1),
				PLUGIN_DEPENDS(PRF, PRF_HMAC_SHA1),
			PLUGIN_PROVIDE(DRBG, DRBG_HMAC_SHA256),
				PLUGIN_DEPENDS(PRF, PRF_HMAC_SHA2_256),
			PLUGIN_PROVIDE(DRBG, DRBG_HMAC_SHA384),
				PLUGIN_DEPENDS(PRF, PRF_HMAC_SHA2_384),
			PLUGIN_PROVIDE(DRBG, DRBG_HMAC_SHA512),
				PLUGIN_DEPENDS(PRF, PRF_HMAC_SHA2_512),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_drbg_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *drbg_plugin_create()
{
	private_drbg_plugin_t *this;

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

/*
 * Copyright (C) 2022 Tobias Brunner
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

#include "kdf_plugin.h"
#include "kdf_kdf.h"

#include <library.h>

typedef struct private_kdf_plugin_t private_kdf_plugin_t;

/**
 * Private data
 */
struct private_kdf_plugin_t {

	/**
	 * Public interface
	 */
	kdf_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_kdf_plugin_t *this)
{
	return "kdf";
}

METHOD(plugin_t, get_features, int,
	private_kdf_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(KDF, kdf_kdf_create),
			PLUGIN_PROVIDE(KDF, KDF_PRF),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA1),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_256),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_384),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_512),
				PLUGIN_SDEPEND(PRF, PRF_AES128_XCBC),
				PLUGIN_SDEPEND(PRF, PRF_AES128_CMAC),
			PLUGIN_PROVIDE(KDF, KDF_PRF_PLUS),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA1),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_256),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_384),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_512),
				PLUGIN_SDEPEND(PRF, PRF_AES128_XCBC),
				PLUGIN_SDEPEND(PRF, PRF_AES128_CMAC),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_kdf_plugin_t *this)
{
	free(this);
}

/*
 * Described in header
 */
plugin_t *kdf_plugin_create()
{
	private_kdf_plugin_t *this;

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

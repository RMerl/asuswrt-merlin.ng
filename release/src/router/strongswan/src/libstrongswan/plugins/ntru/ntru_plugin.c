/*
 * Copyright (C) 2013-2016 Andreas Steffen
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

#include "ntru_plugin.h"
#include "ntru_ke.h"

#include <library.h>

typedef struct private_ntru_plugin_t private_ntru_plugin_t;

/**
 * private data of ntru_plugin
 */
struct private_ntru_plugin_t {

	/**
	 * public functions
	 */
	ntru_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ntru_plugin_t *this)
{
	return "ntru";
}

METHOD(plugin_t, get_features, int,
	private_ntru_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DH, ntru_ke_create),
			PLUGIN_PROVIDE(DH, NTRU_112_BIT),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_SHA2_256_256),
				PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA1),
			PLUGIN_PROVIDE(DH, NTRU_128_BIT),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_SHA2_256_256),
				PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA1),
			PLUGIN_PROVIDE(DH, NTRU_192_BIT),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_SHA2_256_256),
				PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA256),
			PLUGIN_PROVIDE(DH, NTRU_256_BIT),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_SHA2_256_256),
				PLUGIN_DEPENDS(XOF, XOF_MGF1_SHA256),
	};
	*features = f;

	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ntru_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ntru_plugin_create()
{
	private_ntru_plugin_t *this;

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

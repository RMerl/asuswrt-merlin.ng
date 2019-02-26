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

#include "xcbc_plugin.h"

#include <library.h>
#include "xcbc.h"

typedef struct private_xcbc_plugin_t private_xcbc_plugin_t;

/**
 * private data of xcbc_plugin
 */
struct private_xcbc_plugin_t {

	/**
	 * public functions
	 */
	xcbc_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_xcbc_plugin_t *this)
{
	return "xcbc";
}

METHOD(plugin_t, get_features, int,
	private_xcbc_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRF, xcbc_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_AES128_XCBC),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(PRF, PRF_CAMELLIA128_XCBC),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
		PLUGIN_REGISTER(SIGNER, xcbc_signer_create),
			PLUGIN_PROVIDE(SIGNER, AUTH_CAMELLIA_XCBC_96),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(SIGNER, AUTH_AES_XCBC_96),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_xcbc_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *xcbc_plugin_create()
{
	private_xcbc_plugin_t *this;

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


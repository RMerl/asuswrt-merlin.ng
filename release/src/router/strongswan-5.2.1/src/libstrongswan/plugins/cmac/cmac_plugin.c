/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "cmac_plugin.h"

#include <library.h>
#include "cmac.h"

typedef struct private_cmac_plugin_t private_cmac_plugin_t;

/**
 * private data of cmac_plugin
 */
struct private_cmac_plugin_t {

	/**
	 * public functions
	 */
	cmac_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_cmac_plugin_t *this)
{
	return "cmac";
}

METHOD(plugin_t, get_features, int,
	private_cmac_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRF, cmac_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_AES128_CMAC),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
		PLUGIN_REGISTER(SIGNER, cmac_signer_create),
			PLUGIN_PROVIDE(SIGNER, AUTH_AES_CMAC_96),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_cmac_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *cmac_plugin_create()
{
	private_cmac_plugin_t *this;

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


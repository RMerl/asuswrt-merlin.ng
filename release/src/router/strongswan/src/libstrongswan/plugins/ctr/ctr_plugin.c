/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "ctr_plugin.h"

#include <library.h>

#include "ctr_ipsec_crypter.h"

typedef struct private_ctr_plugin_t private_ctr_plugin_t;

/**
 * private data of ctr_plugin
 */
struct private_ctr_plugin_t {

	/**
	 * public functions
	 */
	ctr_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ctr_plugin_t *this)
{
	return "ctr";
}

METHOD(plugin_t, get_features, int,
	private_ctr_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CRYPTER, ctr_ipsec_crypter_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 32),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 32),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ctr_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ctr_plugin_create()
{
	private_ctr_plugin_t *this;

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

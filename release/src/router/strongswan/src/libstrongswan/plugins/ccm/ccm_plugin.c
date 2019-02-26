/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "ccm_plugin.h"

#include <library.h>

#include "ccm_aead.h"

typedef struct private_ccm_plugin_t private_ccm_plugin_t;

/**
 * private data of ccm_plugin
 */
struct private_ccm_plugin_t {

	/**
	 * public functions
	 */
	ccm_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ccm_plugin_t *this)
{
	return "ccm";
}

METHOD(plugin_t, get_features, int,
	private_ccm_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(AEAD, ccm_aead_create),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV8, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV8, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV8, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV12, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV12, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV12, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV16, 16),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV16, 24),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_CAMELLIA_CCM_ICV16, 32),
				PLUGIN_DEPENDS(CRYPTER, ENCR_CAMELLIA_CBC, 32),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ccm_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ccm_plugin_create()
{
	private_ccm_plugin_t *this;

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

/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include "aesni_plugin.h"
#include "aesni_cbc.h"
#include "aesni_ctr.h"
#include "aesni_ccm.h"
#include "aesni_gcm.h"
#include "aesni_xcbc.h"
#include "aesni_cmac.h"

#include <stdio.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/cpu_feature.h>

typedef struct private_aesni_plugin_t private_aesni_plugin_t;
typedef enum cpuid_feature_t cpuid_feature_t;

/**
 * private data of aesni_plugin
 */
struct private_aesni_plugin_t {

	/**
	 * public functions
	 */
	aesni_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_aesni_plugin_t *this)
{
	return "aesni";
}

METHOD(plugin_t, get_features, int,
	private_aesni_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CRYPTER, aesni_cbc_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 32),
		PLUGIN_REGISTER(CRYPTER, aesni_ctr_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 32),
		PLUGIN_REGISTER(AEAD, aesni_ccm_create),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV8,  32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_CCM_ICV16, 32),
		PLUGIN_REGISTER(AEAD, aesni_gcm_create),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8,  32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 32),
		PLUGIN_REGISTER(PRF, aesni_xcbc_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_AES128_XCBC),
		PLUGIN_REGISTER(SIGNER, aesni_xcbc_signer_create),
			PLUGIN_PROVIDE(SIGNER, AUTH_AES_XCBC_96),
		PLUGIN_REGISTER(PRF, aesni_cmac_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_AES128_CMAC),
		PLUGIN_REGISTER(SIGNER, aesni_cmac_signer_create),
			PLUGIN_PROVIDE(SIGNER, AUTH_AES_CMAC_96),
	};

	*features = f;
	if (cpu_feature_available(CPU_FEATURE_AESNI | CPU_FEATURE_PCLMULQDQ))
	{
		return countof(f);
	}
	return 0;
}

METHOD(plugin_t, destroy, void,
	private_aesni_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *aesni_plugin_create()
{
	private_aesni_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = (void*)return_false,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}

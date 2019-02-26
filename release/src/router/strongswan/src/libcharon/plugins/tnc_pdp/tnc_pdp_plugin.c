/*
 * Copyright (C) 2010 Andreas Steffen
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

#include "tnc_pdp_plugin.h"
#include "tnc_pdp.h"

#include <daemon.h>

typedef struct private_tnc_pdp_plugin_t private_tnc_pdp_plugin_t;

/**
 * private data of tnc_pdp plugin
 */
struct private_tnc_pdp_plugin_t {

	/**
	 * implements plugin interface
	 */
	tnc_pdp_plugin_t public;

	/**
	 * Policy Decision Point object
	 */
	tnc_pdp_t *pdp;

};

METHOD(plugin_t, get_name, char*,
	private_tnc_pdp_plugin_t *this)
{
	return "tnc-pdp";
}

/**
 * Register listener
 */
static bool plugin_cb(private_tnc_pdp_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->pdp = tnc_pdp_create();
		if (!this->pdp)
		{
			return FALSE;
		}
	}
	else
	{
		DESTROY_IF(this->pdp);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_tnc_pdp_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "tnc-pdp"),
				PLUGIN_DEPENDS(CUSTOM, "imv-manager"),
				PLUGIN_DEPENDS(HASHER, HASH_MD5),
				PLUGIN_DEPENDS(SIGNER, AUTH_HMAC_MD5_128),
				PLUGIN_DEPENDS(NONCE_GEN),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_tnc_pdp_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *tnc_pdp_plugin_create()
{
	private_tnc_pdp_plugin_t *this;

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


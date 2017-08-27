/*
 * Copyright (C) 2010-2011 Andreas Steffen
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

#include "tnc_imc_plugin.h"
#include "tnc_imc_manager.h"

#include <tnc/tnc.h>

typedef struct private_tnc_imc_plugin_t private_tnc_imc_plugin_t;

/**
 * Private data of a tnc_imc_plugin_t object.
 */
struct private_tnc_imc_plugin_t {

	/**
	 * Public interface.
	 */
	tnc_imc_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_tnc_imc_plugin_t *this)
{
	return "tnc-imc";
}

METHOD(plugin_t, get_features, int,
	private_tnc_imc_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(tnc_manager_register, tnc_imc_manager_create),
			PLUGIN_PROVIDE(CUSTOM, "imc-manager"),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_TRUSTED_PUBKEY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_tnc_imc_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *tnc_imc_plugin_create(void)
{
	private_tnc_imc_plugin_t *this;

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


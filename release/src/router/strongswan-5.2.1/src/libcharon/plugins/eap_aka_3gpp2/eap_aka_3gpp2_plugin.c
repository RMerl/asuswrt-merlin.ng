/*
 * Copyright (C) 2008-2009 Martin Willi
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

#include "eap_aka_3gpp2_plugin.h"
#include "eap_aka_3gpp2_card.h"
#include "eap_aka_3gpp2_provider.h"
#include "eap_aka_3gpp2_functions.h"

#include <daemon.h>

typedef struct private_eap_aka_3gpp2_t private_eap_aka_3gpp2_t;

/**
 * Private data of an eap_aka_3gpp2_t object.
 */
struct private_eap_aka_3gpp2_t {

	/**
	 * Public eap_aka_3gpp2_plugin_t interface.
	 */
	eap_aka_3gpp2_plugin_t public;

	/**
	 * SIM card
	 */
	eap_aka_3gpp2_card_t *card;

	/**
	 * SIM provider
	 */
	eap_aka_3gpp2_provider_t *provider;

	/**
	 * AKA functions
	 */
	eap_aka_3gpp2_functions_t *functions;
};

METHOD(plugin_t, get_name, char*,
	private_eap_aka_3gpp2_t *this)
{
	return "eap-aka-3gpp2";
}

/**
 * Try to instanciate 3gpp2 functions and card/provider backends
 */
static bool register_functions(private_eap_aka_3gpp2_t *this,
							   plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		this->functions = eap_aka_3gpp2_functions_create();
		if (!this->functions)
		{
			return FALSE;
		}
		this->card = eap_aka_3gpp2_card_create(this->functions);
		this->provider = eap_aka_3gpp2_provider_create(this->functions);
		return TRUE;
	}
	this->card->destroy(this->card);
	this->provider->destroy(this->provider);
	this->functions->destroy(this->functions);
	this->card = NULL;
	this->provider = NULL;
	this->functions = NULL;
	return TRUE;
}

/**
 * Callback providing our card to register
 */
static simaka_card_t* get_card(private_eap_aka_3gpp2_t *this)
{
	return &this->card->card;
}

/**
 * Callback providing our provider to register
 */
static simaka_provider_t* get_provider(private_eap_aka_3gpp2_t *this)
{
	return &this->provider->provider;
}

METHOD(plugin_t, get_features, int,
	private_eap_aka_3gpp2_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((void*)register_functions, NULL),
			PLUGIN_PROVIDE(CUSTOM, "eap-aka-3gpp2-functions"),
				PLUGIN_DEPENDS(PRF, PRF_KEYED_SHA1),
		PLUGIN_CALLBACK(simaka_manager_register, get_card),
			PLUGIN_PROVIDE(CUSTOM, "aka-card"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-aka-3gpp2-functions"),
		PLUGIN_CALLBACK(simaka_manager_register, get_provider),
			PLUGIN_PROVIDE(CUSTOM, "aka-provider"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-aka-3gpp2-functions"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_eap_aka_3gpp2_t *this)
{
	free(this);
}

/**
 * See header
 */
plugin_t *eap_aka_3gpp2_plugin_create()
{
	private_eap_aka_3gpp2_t *this;

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


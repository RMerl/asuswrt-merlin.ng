/*
 * Copyright (C) 2009 Martin Willi
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

#include "eap_simaka_reauth_plugin.h"
#include "eap_simaka_reauth_card.h"
#include "eap_simaka_reauth_provider.h"

#include <daemon.h>

typedef struct private_eap_simaka_reauth_t private_eap_simaka_reauth_t;

/**
 * Private data of an eap_simaka_reauth_t object.
 */
struct private_eap_simaka_reauth_t {

	/**
	 * Public eap_simaka_reauth_plugin_t interface.
	 */
	eap_simaka_reauth_plugin_t public;

	/**
	 * SIM card
	 */
	eap_simaka_reauth_card_t *card;

	/**
	 * SIM provider
	 */
	eap_simaka_reauth_provider_t *provider;
};

METHOD(plugin_t, get_name, char*,
	private_eap_simaka_reauth_t *this)
{
	return "eap-simaka-reauth";
}

/**
 * Callback providing our card to register
 */
static simaka_card_t* get_card(private_eap_simaka_reauth_t *this)
{
	if (!this->card)
	{
		this->card = eap_simaka_reauth_card_create();
	}
	return &this->card->card;
}

/**
 * Callback providing our provider to register
 */
static simaka_provider_t* get_provider(private_eap_simaka_reauth_t *this)
{
	if (!this->provider)
	{
		this->provider = eap_simaka_reauth_provider_create();
		if (!this->provider)
		{
			return NULL;
		}
	}
	return &this->provider->provider;
}

METHOD(plugin_t, get_features, int,
	private_eap_simaka_reauth_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(simaka_manager_register, get_card),
			PLUGIN_PROVIDE(CUSTOM, "aka-card"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
			PLUGIN_PROVIDE(CUSTOM, "sim-card"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
		PLUGIN_CALLBACK(simaka_manager_register, get_provider),
			PLUGIN_PROVIDE(CUSTOM, "aka-provider"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
			PLUGIN_PROVIDE(CUSTOM, "sim-provider"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_eap_simaka_reauth_t *this)
{
	DESTROY_IF(this->card);
	DESTROY_IF(this->provider);
	free(this);
}

/**
 * See header
 */
plugin_t *eap_simaka_reauth_plugin_create()
{
	private_eap_simaka_reauth_t *this;

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


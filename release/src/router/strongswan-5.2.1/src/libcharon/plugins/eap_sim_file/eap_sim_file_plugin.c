/*
 * Copyright (C) 2008 Martin Willi
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

#include "eap_sim_file_plugin.h"
#include "eap_sim_file_card.h"
#include "eap_sim_file_provider.h"
#include "eap_sim_file_triplets.h"

#include <daemon.h>

#define TRIPLET_FILE IPSEC_CONFDIR "/ipsec.d/triplets.dat"

typedef struct private_eap_sim_file_t private_eap_sim_file_t;

/**
 * Private data of an eap_sim_file_t object.
 */
struct private_eap_sim_file_t {

	/**
	 * Public eap_sim_file_plugin_t interface.
	 */
	eap_sim_file_plugin_t public;

	/**
	 * SIM card
	 */
	eap_sim_file_card_t *card;

	/**
	 * SIM provider
	 */
	eap_sim_file_provider_t *provider;

	/**
	 * Triplet source
	 */
	eap_sim_file_triplets_t *triplets;
};

METHOD(plugin_t, get_name, char*,
	private_eap_sim_file_t *this)
{
	return "eap-sim-file";
}

/**
 * Load triplet file
 */
static bool load_triplets(private_eap_sim_file_t *this,
						  plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		this->triplets = eap_sim_file_triplets_create(TRIPLET_FILE);
		if (!this->triplets)
		{
			return FALSE;
		}
		this->provider = eap_sim_file_provider_create(this->triplets);
		this->card = eap_sim_file_card_create(this->triplets);
		return TRUE;
	}
	this->card->destroy(this->card);
	this->provider->destroy(this->provider);
	this->triplets->destroy(this->triplets);
	this->card = NULL;
	this->provider = NULL;
	this->triplets = NULL;
	return TRUE;
}

/**
 * Callback providing our card to register
 */
static simaka_card_t* get_card(private_eap_sim_file_t *this)
{
	return &this->card->card;
}

/**
 * Callback providing our provider to register
 */
static simaka_provider_t* get_provider(private_eap_sim_file_t *this)
{
	return &this->provider->provider;
}

METHOD(plugin_t, get_features, int,
	private_eap_sim_file_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((void*)load_triplets, NULL),
			PLUGIN_PROVIDE(CUSTOM, "eap-sim-file-triplets"),
		PLUGIN_CALLBACK(simaka_manager_register, get_card),
			PLUGIN_PROVIDE(CUSTOM, "sim-card"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-sim-file-triplets"),
		PLUGIN_CALLBACK(simaka_manager_register, get_provider),
			PLUGIN_PROVIDE(CUSTOM, "sim-provider"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-sim-file-triplets"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_eap_sim_file_t *this)
{
	free(this);
}

/**
 * See header
 */
plugin_t *eap_sim_file_plugin_create()
{
	private_eap_sim_file_t *this;

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


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

#include "eap_simaka_sql_plugin.h"
#include "eap_simaka_sql_card.h"
#include "eap_simaka_sql_provider.h"

#include <daemon.h>

typedef struct private_eap_simaka_sql_t private_eap_simaka_sql_t;

/**
 * Private data of an eap_simaka_sql_t object.
 */
struct private_eap_simaka_sql_t {

	/**
	 * Public eap_simaka_sql_plugin_t interface.
	 */
	eap_simaka_sql_plugin_t public;

	/**
	 * (U)SIM card
	 */
	eap_simaka_sql_card_t *card;

	/**
	 * (U)SIM provider
	 */
	eap_simaka_sql_provider_t *provider;

	/**
	 * Database with triplets/quintuplets
	 */
	database_t *db;
};

METHOD(plugin_t, get_name, char*,
	private_eap_simaka_sql_t *this)
{
	return "eap-simaka-sql";
}

/**
 * Load database
 */
static bool load_db(private_eap_simaka_sql_t *this,
					plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		bool remove_used;
		char *uri;

		uri = lib->settings->get_str(lib->settings,
									 "%s.plugins.eap-simaka-sql.database", NULL,
									 lib->ns);
		if (!uri)
		{
			DBG1(DBG_CFG, "eap-simaka-sql database URI missing");
			return FALSE;
		}
		this->db = lib->db->create(lib->db, uri);
		if (!this->db)
		{
			DBG1(DBG_CFG, "opening eap-simaka-sql database failed");
			return FALSE;
		}
		remove_used = lib->settings->get_bool(lib->settings,
								"%s.plugins.eap-simaka-sql.remove_used", FALSE,
								lib->ns);

		this->provider = eap_simaka_sql_provider_create(this->db, remove_used);
		this->card = eap_simaka_sql_card_create(this->db, remove_used);
		return TRUE;
	}
	this->card->destroy(this->card);
	this->provider->destroy(this->provider);
	this->db->destroy(this->db);
	this->card = NULL;
	this->provider = NULL;
	this->db = NULL;
	return TRUE;
}

/**
 * Callback providing our card to register
 */
static simaka_card_t* get_card(private_eap_simaka_sql_t *this)
{
	return &this->card->card;
}

/**
 * Callback providing our provider to register
 */
static simaka_provider_t* get_provider(private_eap_simaka_sql_t *this)
{
	return &this->provider->provider;
}

METHOD(plugin_t, get_features, int,
	private_eap_simaka_sql_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((void*)load_db, NULL),
			PLUGIN_PROVIDE(CUSTOM, "eap-simaka-sql-db"),
				PLUGIN_DEPENDS(DATABASE, DB_ANY),
				PLUGIN_SDEPEND(DATABASE, DB_SQLITE),
				PLUGIN_SDEPEND(DATABASE, DB_MYSQL),
		PLUGIN_CALLBACK(simaka_manager_register, get_card),
			PLUGIN_PROVIDE(CUSTOM, "aka-card"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-simaka-sql-db"),
			PLUGIN_PROVIDE(CUSTOM, "sim-card"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-simaka-sql-db"),
		PLUGIN_CALLBACK(simaka_manager_register, get_provider),
			PLUGIN_PROVIDE(CUSTOM, "aka-provider"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-simaka-sql-db"),
			PLUGIN_PROVIDE(CUSTOM, "sim-provider"),
				PLUGIN_DEPENDS(CUSTOM, "sim-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-simaka-sql-db"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_eap_simaka_sql_t *this)
{
	free(this);
}

/**
 * See header
 */
plugin_t *eap_simaka_sql_plugin_create()
{
	private_eap_simaka_sql_t *this;

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

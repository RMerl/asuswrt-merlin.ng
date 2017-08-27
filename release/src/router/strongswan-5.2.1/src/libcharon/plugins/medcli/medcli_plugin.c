/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "medcli_plugin.h"

#include "medcli_creds.h"
#include "medcli_config.h"
#include "medcli_listener.h"

#include <daemon.h>

typedef struct private_medcli_plugin_t private_medcli_plugin_t;

/**
 * private data of medcli plugin
 */
struct private_medcli_plugin_t {

	/**
	 * implements plugin interface
	 */
	medcli_plugin_t public;

	/**
	 * database connection instance
	 */
	database_t *db;

	/**
	 * medcli credential set instance
	 */
	medcli_creds_t *creds;

	/**
	 * medcli config database
	 */
	medcli_config_t *config;

	/**
	 * Listener to update database connection state
	 */
	medcli_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_medcli_plugin_t *this)
{
	return "medcli";
}

/**
 * Connect to database
 */
static bool open_database(private_medcli_plugin_t *this,
						  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		char *uri;

		uri = lib->settings->get_str(lib->settings,
									 "medcli.database", NULL);
		if (!uri)
		{
			DBG1(DBG_CFG, "mediation client database URI not defined, skipped");
			return FALSE;
		}

		this->db = lib->db->create(lib->db, uri);
		if (this->db == NULL)
		{
			DBG1(DBG_CFG, "opening mediation client database failed");
			return FALSE;
		}

		this->creds = medcli_creds_create(this->db);
		this->config = medcli_config_create(this->db);
		this->listener = medcli_listener_create(this->db);

		lib->credmgr->add_set(lib->credmgr, &this->creds->set);
		charon->backends->add_backend(charon->backends, &this->config->backend);
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
		charon->backends->remove_backend(charon->backends, &this->config->backend);
		lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
		this->listener->destroy(this->listener);
		this->config->destroy(this->config);
		this->creds->destroy(this->creds);
		this->db->destroy(this->db);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_medcli_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)open_database, NULL),
			PLUGIN_PROVIDE(CUSTOM, "medcli"),
				PLUGIN_DEPENDS(DATABASE, DB_ANY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_medcli_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *medcli_plugin_create()
{
	private_medcli_plugin_t *this;

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

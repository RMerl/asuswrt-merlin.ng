/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include "sql_plugin.h"

#include <daemon.h>
#include <plugins/plugin_feature.h>

#include "sql_config.h"
#include "sql_cred.h"
#include "sql_logger.h"

typedef struct private_sql_plugin_t private_sql_plugin_t;

/**
 * private data of sql plugin
 */
struct private_sql_plugin_t {

	/**
	 * implements plugin interface
	 */
	sql_plugin_t public;

	/**
	 * database connection instance
	 */
	database_t *db;

	/**
	 * configuration backend
	 */
	sql_config_t *config;

	/**
	 * credential set
	 */
	sql_cred_t *cred;

	/**
	 * bus listener/logger
	 */
	sql_logger_t *logger;
};

METHOD(plugin_t, get_name, char*,
	private_sql_plugin_t *this)
{
	return "sql";
}

/**
 * Connect to database
 */
static bool open_database(private_sql_plugin_t *this,
						  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		char *uri;

		uri = lib->settings->get_str(lib->settings, "%s.plugins.sql.database",
									 NULL, lib->ns);
		if (!uri)
		{
			DBG1(DBG_CFG, "sql plugin: database URI not set");
			return FALSE;
		}

		this->db = lib->db->create(lib->db, uri);
		if (!this->db)
		{
			DBG1(DBG_CFG, "sql plugin failed to connect to database");
			return FALSE;
		}
		this->config = sql_config_create(this->db);
		this->cred = sql_cred_create(this->db);
		this->logger = sql_logger_create(this->db);

		charon->backends->add_backend(charon->backends, &this->config->backend);
		lib->credmgr->add_set(lib->credmgr, &this->cred->set);
		charon->bus->add_logger(charon->bus, &this->logger->logger);
	}
	else
	{
		charon->backends->remove_backend(charon->backends,
										 &this->config->backend);
		lib->credmgr->remove_set(lib->credmgr, &this->cred->set);
		charon->bus->remove_logger(charon->bus, &this->logger->logger);
		this->config->destroy(this->config);
		this->cred->destroy(this->cred);
		this->logger->destroy(this->logger);
		this->db->destroy(this->db);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_sql_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)open_database, NULL),
			PLUGIN_PROVIDE(CUSTOM, "sql"),
				PLUGIN_DEPENDS(DATABASE, DB_ANY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_sql_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *sql_plugin_create()
{
	private_sql_plugin_t *this;

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

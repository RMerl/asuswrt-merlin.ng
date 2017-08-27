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

#include "mysql_plugin.h"

#include <library.h>
#include <utils/debug.h>
#include "mysql_database.h"

typedef struct private_mysql_plugin_t private_mysql_plugin_t;

/**
 * private data of mysql_plugin
 */
struct private_mysql_plugin_t {

	/**
	 * public functions
	 */
	mysql_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_mysql_plugin_t *this)
{
	return "mysql";
}

METHOD(plugin_t, get_features, int,
	private_mysql_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DATABASE, mysql_database_create),
			PLUGIN_PROVIDE(DATABASE, DB_MYSQL),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_mysql_plugin_t *this)
{
	mysql_database_deinit();
	free(this);
}

/*
 * see header file
 */
plugin_t *mysql_plugin_create()
{
	private_mysql_plugin_t *this;

	if (!mysql_database_init())
	{
		DBG1(DBG_LIB, "MySQL client library initialization failed");
		return NULL;
	}

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


/*
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

#include "sqlite_plugin.h"

#include <library.h>
#include <sqlite3.h>
#include "sqlite_database.h"

typedef struct private_sqlite_plugin_t private_sqlite_plugin_t;

/**
 * private data of sqlite_plugin
 */
struct private_sqlite_plugin_t {

	/**
	 * public functions
	 */
	sqlite_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_sqlite_plugin_t *this)
{
	return "sqlite";
}

METHOD(plugin_t, get_features, int,
	private_sqlite_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(DATABASE, sqlite_database_create),
			PLUGIN_PROVIDE(DATABASE, DB_SQLITE),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_sqlite_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *sqlite_plugin_create()
{
	private_sqlite_plugin_t *this;
	int threadsafe = 0;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

#if SQLITE_VERSION_NUMBER >= 3005000
	threadsafe = sqlite3_threadsafe();
#endif
	DBG2(DBG_LIB, "using SQLite %s, thread safety %d",
		 sqlite3_libversion(), threadsafe);

	return &this->public.plugin;
}

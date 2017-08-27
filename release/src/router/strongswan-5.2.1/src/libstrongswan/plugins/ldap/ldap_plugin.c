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

#include "ldap_plugin.h"

#include <library.h>
#include "ldap_fetcher.h"

typedef struct private_ldap_plugin_t private_ldap_plugin_t;

/**
 * private data of ldap_plugin
 */
struct private_ldap_plugin_t {

	/**
	 * public functions
	 */
	ldap_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_ldap_plugin_t *this)
{
	return "ldap";
}

METHOD(plugin_t, get_features, int,
	private_ldap_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(FETCHER, ldap_fetcher_create),
			PLUGIN_PROVIDE(FETCHER, "ldap://"),
			PLUGIN_PROVIDE(FETCHER, "ldaps://"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ldap_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *ldap_plugin_create()
{
	private_ldap_plugin_t *this;

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


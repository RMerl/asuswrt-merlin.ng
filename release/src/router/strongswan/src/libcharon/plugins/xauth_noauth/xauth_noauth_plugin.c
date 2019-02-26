/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "xauth_noauth_plugin.h"
#include "xauth_noauth.h"

#include <daemon.h>

METHOD(plugin_t, get_name, char*,
	xauth_noauth_plugin_t *this)
{
	return "xauth-noauth";
}

METHOD(plugin_t, get_features, int,
	xauth_noauth_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(xauth_method_register, xauth_noauth_create_server),
			PLUGIN_PROVIDE(XAUTH_SERVER, "noauth"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	xauth_noauth_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *xauth_noauth_plugin_create()
{
	xauth_noauth_plugin_t *this;

	INIT(this,
		.plugin = {
			.get_name = _get_name,
			.get_features = _get_features,
			.destroy = _destroy,
		},
	);

	return &this->plugin;
}

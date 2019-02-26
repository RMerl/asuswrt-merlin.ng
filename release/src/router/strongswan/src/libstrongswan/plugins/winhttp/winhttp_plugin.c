/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "winhttp_plugin.h"
#include "winhttp_fetcher.h"

typedef struct private_winhttp_plugin_t private_winhttp_plugin_t;

/**
 * Private data of winhttp_plugin
 */
struct private_winhttp_plugin_t {

	/**
	 * Public functions
	 */
	winhttp_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_winhttp_plugin_t *this)
{
	return "winhttp";
}

METHOD(plugin_t, get_features, int,
	private_winhttp_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(FETCHER, winhttp_fetcher_create),
			PLUGIN_PROVIDE(FETCHER, "http://"),
			PLUGIN_PROVIDE(FETCHER, "https://"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_winhttp_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *winhttp_plugin_create()
{
	private_winhttp_plugin_t *this;

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

/*
 * Copyright (C) 2008 Martin Willi
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

#include "sha2_plugin.h"

#include <library.h>
#include "sha2_hasher.h"

typedef struct private_sha2_plugin_t private_sha2_plugin_t;

/**
 * private data of sha2_plugin
 */
struct private_sha2_plugin_t {

	/**
	 * public functions
	 */
	sha2_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_sha2_plugin_t *this)
{
	return "sha2";
}

METHOD(plugin_t, get_features, int,
	private_sha2_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(HASHER, sha2_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_SHA224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_sha2_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *sha2_plugin_create()
{
	private_sha2_plugin_t *this;

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


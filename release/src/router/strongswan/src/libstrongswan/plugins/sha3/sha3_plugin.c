/*
 * Copyright (C) 2015 Andreas Steffen
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

#include "sha3_plugin.h"
#include "sha3_hasher.h"
#include "sha3_shake.h"

#include <library.h>

typedef struct private_sha3_plugin_t private_sha3_plugin_t;

/**
 * private data of sha3_plugin
 */
struct private_sha3_plugin_t {

	/**
	 * public functions
	 */
	sha3_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_sha3_plugin_t *this)
{
	return "sha3";
}

METHOD(plugin_t, get_features, int,
	private_sha3_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(HASHER, sha3_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_256),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA3_512),
		PLUGIN_REGISTER(XOF, sha3_shake_create),
			PLUGIN_PROVIDE(XOF, XOF_SHAKE_128),
			PLUGIN_PROVIDE(XOF, XOF_SHAKE_256),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_sha3_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *sha3_plugin_create()
{
	private_sha3_plugin_t *this;

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


/*
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include "nonce_plugin.h"

#include <library.h>
#include "nonce_nonceg.h"

typedef struct private_nonce_plugin_t private_nonce_plugin_t;

/**
 * private data of nonce_plugin
 */
struct private_nonce_plugin_t {

	/**
	 * public functions
	 */
	nonce_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_nonce_plugin_t *this)
{
	return "nonce";
}

METHOD(plugin_t, get_features, int,
	private_nonce_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(NONCE_GEN, nonce_nonceg_create),
			PLUGIN_PROVIDE(NONCE_GEN),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_nonce_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *nonce_plugin_create()
{
	private_nonce_plugin_t *this;

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

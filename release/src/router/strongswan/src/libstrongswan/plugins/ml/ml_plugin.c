/*
 * Copyright (C) 2024 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "ml_plugin.h"

#include <plugins/plugin.h>

#include "ml_kem.h"

typedef struct private_plugin_t private_plugin_t;

/**
 * Private data.
 */
struct private_plugin_t {

	/**
	 * Public interface.
	 */
	plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_plugin_t *this)
{
	return "ml";
}

METHOD(plugin_t, get_features, int,
	private_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(KE, ml_kem_create),
			PLUGIN_PROVIDE(KE, ML_KEM_512),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_256),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_512),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_128),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_256),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(KE, ML_KEM_768),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_256),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_512),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_128),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_256),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(KE, ML_KEM_1024),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_256),
				PLUGIN_DEPENDS(HASHER, HASH_SHA3_512),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_128),
				PLUGIN_DEPENDS(XOF, XOF_SHAKE_256),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_plugin_t *this)
{
	free(this);
}

/*
 * Described in header
 */
PLUGIN_DEFINE(ml)
{
	private_plugin_t *this;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_features = _get_features,
			.destroy = _destroy,
		},
	);

	return &this->public;
}

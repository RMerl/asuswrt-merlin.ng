/*
 * Copyright (C) 2016 Andreas Steffen
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

#include "mgf1_plugin.h"
#include "mgf1_xof.h"

#include <library.h>

typedef struct private_mgf1_plugin_t private_mgf1_plugin_t;

/**
 * private data of mgf1_plugin
 */
struct private_mgf1_plugin_t {

	/**
	 * public functions
	 */
	mgf1_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_mgf1_plugin_t *this)
{
	return "mgf1";
}

METHOD(plugin_t, get_features, int,
	private_mgf1_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(XOF, mgf1_xof_create),
			PLUGIN_PROVIDE(XOF, XOF_MGF1_SHA1),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
			PLUGIN_PROVIDE(XOF, XOF_MGF1_SHA224),
				PLUGIN_DEPENDS(HASHER, HASH_SHA224),
			PLUGIN_PROVIDE(XOF, XOF_MGF1_SHA256),
				PLUGIN_DEPENDS(HASHER, HASH_SHA256),
			PLUGIN_PROVIDE(XOF, XOF_MGF1_SHA384),
				PLUGIN_DEPENDS(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(XOF, XOF_MGF1_SHA512),
				PLUGIN_DEPENDS(HASHER, HASH_SHA512),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_mgf1_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *mgf1_plugin_create()
{
	private_mgf1_plugin_t *this;

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


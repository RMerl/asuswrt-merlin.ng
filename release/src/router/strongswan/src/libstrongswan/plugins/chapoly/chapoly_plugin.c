/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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

#include "chapoly_plugin.h"
#include "chapoly_aead.h"
#include "chapoly_xof.h"

#include <library.h>

typedef struct private_chapoly_plugin_t private_chapoly_plugin_t;

/**
 * Private data of chapoly_plugin
 */
struct private_chapoly_plugin_t {

	/**
	 * Public functions
	 */
	chapoly_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_chapoly_plugin_t *this)
{
	return "chapoly";
}

METHOD(plugin_t, get_features, int,
	private_chapoly_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(AEAD, chapoly_aead_create),
			PLUGIN_PROVIDE(AEAD, ENCR_CHACHA20_POLY1305, 32),
		PLUGIN_REGISTER(XOF, chapoly_xof_create),
			PLUGIN_PROVIDE(XOF, XOF_CHACHA20),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_chapoly_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *chapoly_plugin_create()
{
	private_chapoly_plugin_t *this;

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

/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "keychain_plugin.h"
#include "keychain_creds.h"

#include <library.h>

typedef struct private_keychain_plugin_t private_keychain_plugin_t;

/**
 * private data of keychain_plugin
 */
struct private_keychain_plugin_t {

	/**
	 * public functions
	 */
	keychain_plugin_t public;

	/**
	 * System level Keychain Services credential set
	 */
	keychain_creds_t *creds;
};

METHOD(plugin_t, get_name, char*,
	private_keychain_plugin_t *this)
{
	return "keychain";
}

/**
 * Load/unload certificates from Keychain.
 */
static bool load_creds(private_keychain_plugin_t *this,
					   plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		this->creds = keychain_creds_create();
	}
	else
	{
		this->creds->destroy(this->creds);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_keychain_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)load_creds, NULL),
			PLUGIN_PROVIDE(CUSTOM, "keychain"),
				PLUGIN_DEPENDS(CERT_DECODE, CERT_X509),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_keychain_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *keychain_plugin_create()
{
	private_keychain_plugin_t *this;

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

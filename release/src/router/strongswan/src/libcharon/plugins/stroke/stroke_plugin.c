/*
 * Copyright (C) 2008 Martin Willi
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

#include "stroke_plugin.h"

#include <library.h>
#include "stroke_socket.h"

typedef struct private_stroke_plugin_t private_stroke_plugin_t;

/**
 * private data of stroke_plugin
 */
struct private_stroke_plugin_t {

	/**
	 * public functions
	 */
	stroke_plugin_t public;

	/**
	 * stroke socket, receives strokes
	 */
	stroke_socket_t *socket;
};

METHOD(plugin_t, get_name, char*,
	private_stroke_plugin_t *this)
{
	return "stroke";
}

/**
 * Register stroke plugin features
 */
static bool register_stroke(private_stroke_plugin_t *this,
							plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		this->socket = stroke_socket_create();
		return this->socket != NULL;
	}
	else
	{
		DESTROY_IF(this->socket);
		return TRUE;
	}
}

METHOD(plugin_t, get_features, int,
	private_stroke_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)register_stroke, NULL),
			PLUGIN_PROVIDE(CUSTOM, "stroke"),
				PLUGIN_SDEPEND(CUSTOM, "counters"),
				PLUGIN_SDEPEND(PRIVKEY, KEY_RSA),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ECDSA),
				PLUGIN_SDEPEND(PRIVKEY, KEY_DSA),
				PLUGIN_SDEPEND(PRIVKEY, KEY_BLISS),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ED25519),
				PLUGIN_SDEPEND(PRIVKEY, KEY_ED448),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_ANY),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509_CRL),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509_AC),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_TRUSTED_PUBKEY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_stroke_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *stroke_plugin_create()
{
	private_stroke_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.reload = (void*)return_false,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}

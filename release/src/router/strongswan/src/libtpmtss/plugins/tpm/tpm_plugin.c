/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "tpm_plugin.h"
#include "tpm_private_key.h"
#include "tpm_cert.h"
#include "tpm_rng.h"

#include <tpm_tss.h>
#include <library.h>

typedef struct private_tpm_plugin_t private_tpm_plugin_t;

/**
 * private data of tpm_plugin
 */
struct private_tpm_plugin_t {

	/**
	 * public functions
	 */
	tpm_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_tpm_plugin_t *this)
{
	return "tpm";
}

METHOD(plugin_t, get_features, int,
	private_tpm_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f_rng[] = {
		PLUGIN_REGISTER(RNG, tpm_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
	};
	static plugin_feature_t f_privkey[] = {
		PLUGIN_REGISTER(PRIVKEY, tpm_private_key_connect, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
	};
	static plugin_feature_t f_cert[] = {
		PLUGIN_REGISTER(CERT_DECODE, tpm_cert_load, FALSE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509),
				PLUGIN_DEPENDS(CERT_DECODE, CERT_X509),
	};
	static plugin_feature_t f[countof(f_rng) + countof(f_privkey) +
							  countof(f_cert)] = {};
	static int count = 0;

	if (!count)
	{
		plugin_features_add(f, f_privkey, countof(f_privkey), &count);
		plugin_features_add(f, f_cert, countof(f_cert), &count);

		if (lib->settings->get_bool(lib->settings,
								"%s.plugins.tpm.use_rng", FALSE, lib->ns))
		{
			plugin_features_add(f, f_rng, countof(f_rng), &count);
		}
	}
	*features = f;

	return count;
}

METHOD(plugin_t, destroy, void,
	private_tpm_plugin_t *this)
{
	free(this);
	libtpmtss_deinit();
}

/*
 * see header file
 */
plugin_t *tpm_plugin_create()
{
	private_tpm_plugin_t *this;

	if (!libtpmtss_init())
	{
		return NULL;
	}

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

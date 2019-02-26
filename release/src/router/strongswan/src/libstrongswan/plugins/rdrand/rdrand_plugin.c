/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "rdrand_plugin.h"
#include "rdrand_rng.h"

#include <stdio.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/cpu_feature.h>

typedef struct private_rdrand_plugin_t private_rdrand_plugin_t;
typedef enum cpuid_feature_t cpuid_feature_t;

/**
 * private data of rdrand_plugin
 */
struct private_rdrand_plugin_t {

	/**
	 * public functions
	 */
	rdrand_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_rdrand_plugin_t *this)
{
	return "rdrand";
}

METHOD(plugin_t, get_features, int,
	private_rdrand_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(RNG, rdrand_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
				PLUGIN_DEPENDS(CRYPTER, ENCR_AES_CBC, 16),
	};
	*features = f;
	if (cpu_feature_available(CPU_FEATURE_RDRAND))
	{
		DBG2(DBG_LIB, "detected RDRAND support, enabled");
		return countof(f);
	}
	DBG2(DBG_LIB, "no RDRAND support detected, disabled");
	return 0;
}

METHOD(plugin_t, destroy, void,
	private_rdrand_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *rdrand_plugin_create()
{
	private_rdrand_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = (void*)return_false,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}

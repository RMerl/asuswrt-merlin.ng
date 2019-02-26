/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "padlock_plugin.h"
#include "padlock_aes_crypter.h"
#include "padlock_sha1_hasher.h"
#include "padlock_rng.h"

#include <stdio.h>

#include <library.h>
#include <plugins/plugin_feature.h>
#include <utils/cpu_feature.h>
#include <utils/debug.h>

typedef struct private_padlock_plugin_t private_padlock_plugin_t;
typedef enum padlock_feature_t padlock_feature_t;

/**
 * private data of aes_plugin
 */
struct private_padlock_plugin_t {

	/**
	 * public functions
	 */
	padlock_plugin_t public;

	/**
	 * features supported by Padlock
	 */
	cpu_feature_t features;
};

METHOD(plugin_t, get_name, char*,
	private_padlock_plugin_t *this)
{
	return "padlock";
}

METHOD(plugin_t, get_features, int,
	private_padlock_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f_rng[] = {
		PLUGIN_REGISTER(RNG, padlock_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
	};
	static plugin_feature_t f_aes[] = {
		PLUGIN_REGISTER(CRYPTER, padlock_aes_crypter_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
	};
	static plugin_feature_t f_sha1[] = {
		PLUGIN_REGISTER(HASHER, padlock_sha1_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
	};
	static plugin_feature_t f[countof(f_rng) + countof(f_aes) +
							  countof(f_sha1)] = {};
	static int count = 0;

	if (!count)
	{	/* initialize only once */
		if (this->features & CPU_FEATURE_PADLOCK_RNG_ENABLED)
		{
			plugin_features_add(f, f_rng, countof(f_rng), &count);
		}
		if (this->features & CPU_FEATURE_PADLOCK_ACE2_ENABLED)
		{
			plugin_features_add(f, f_aes, countof(f_aes), &count);
		}
		if (this->features & CPU_FEATURE_PADLOCK_PHE_ENABLED)
		{
			plugin_features_add(f, f_sha1, countof(f_sha1), &count);
		}
	}
	*features = f;
	return count;
}

METHOD(plugin_t, destroy, void,
	private_padlock_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *padlock_plugin_create()
{
	private_padlock_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.features = cpu_feature_get_all(),
	);

	DBG1(DBG_LIB, "Padlock features supported:%s%s%s%s%s, enabled:%s%s%s%s%s",
		 this->features & CPU_FEATURE_PADLOCK_RNG_AVAILABLE ? " RNG" : "",
		 this->features & CPU_FEATURE_PADLOCK_ACE_AVAILABLE ? " ACE" : "",
		 this->features & CPU_FEATURE_PADLOCK_ACE2_AVAILABLE ? " ACE2" : "",
		 this->features & CPU_FEATURE_PADLOCK_PHE_AVAILABLE ? " PHE" : "",
		 this->features & CPU_FEATURE_PADLOCK_PMM_AVAILABLE ? " PMM" : "",
		 this->features & CPU_FEATURE_PADLOCK_RNG_ENABLED ? " RNG" : "",
		 this->features & CPU_FEATURE_PADLOCK_ACE_ENABLED ? " ACE" : "",
		 this->features & CPU_FEATURE_PADLOCK_ACE2_ENABLED ? " ACE2" : "",
		 this->features & CPU_FEATURE_PADLOCK_PHE_ENABLED ? " PHE" : "",
		 this->features & CPU_FEATURE_PADLOCK_PMM_ENABLED ? " PMM" : "");

	return &this->public.plugin;
}

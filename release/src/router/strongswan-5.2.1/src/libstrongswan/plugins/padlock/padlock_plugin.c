/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
#include <utils/debug.h>

typedef struct private_padlock_plugin_t private_padlock_plugin_t;
typedef enum padlock_feature_t padlock_feature_t;

/**
 * Feature flags of padlock, received via cpuid()
 */
enum padlock_feature_t {
	PADLOCK_RESERVED_1 = 		(1<<0),
	PADLOCK_RESERVED_2 = 		(1<<1),
	PADLOCK_RNG_AVAILABLE = 	(1<<2),
	PADLOCK_RNG_ENABLED = 		(1<<3),
	PADLOCK_RESERVED_3 = 		(1<<4),
	PADLOCK_RESERVED_4 = 		(1<<5),
	PADLOCK_ACE_AVAILABLE = 	(1<<6),
	PADLOCK_ACE_ENABLED = 		(1<<7),
	PADLOCK_ACE2_AVAILABLE = 	(1<<8),
	PADLOCK_ACE2_ENABLED = 		(1<<9),
	PADLOCK_PHE_AVAILABLE = 	(1<<10),
	PADLOCK_PHE_ENABLED = 		(1<<11),
	PADLOCK_PMM_AVAILABLE = 	(1<<12),
	PADLOCK_PMM_ENABLED = 		(1<<13),
};

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
	padlock_feature_t features;
};

/**
 * Get cpuid for info, return eax, ebx, ecx and edx. -fPIC requires to save ebx.
 */
#define cpuid(op, a, b, c, d)\
	asm (\
		"pushl %%ebx		\n\t"\
		"cpuid				\n\t"\
		"movl %%ebx, %1		\n\t"\
		"popl %%ebx			\n\t"\
		: "=a" (a), "=r" (b), "=c" (c), "=d" (d) \
		: "a" (op));

/**
 * Get features supported by Padlock
 */
static padlock_feature_t get_padlock_features()
{
	char vendor[3 * sizeof(int) + 1];
	int a, b, c, d;

	cpuid(0, a, b, c, d);
	/* VendorID string is in b-d-c (yes, in this order) */
	snprintf(vendor, sizeof(vendor), "%.4s%.4s%.4s", &b, &d, &c);

	/* check if we have a VIA chip */
	if (streq(vendor, "CentaurHauls"))
	{
		cpuid(0xC0000000, a, b, c, d);
		/* check Centaur Extended Feature Flags */
		if (a >= 0xC0000001)
		{
			cpuid(0xC0000001, a, b, c, d);
			return d;
		}
	}
	DBG1(DBG_LIB, "Padlock not found, CPU is %s", vendor);
	return 0;
}

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
		if (this->features & PADLOCK_RNG_ENABLED)
		{
			plugin_features_add(f, f_rng, countof(f_rng), &count);
		}
		if (this->features & PADLOCK_ACE2_ENABLED)
		{
			plugin_features_add(f, f_aes, countof(f_aes), &count);
		}
		if (this->features & PADLOCK_PHE_ENABLED)
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
		.features = get_padlock_features(),
	);

	if (!this->features)
	{
		free(this);
		return NULL;
	}
	DBG1(DBG_LIB, "Padlock found, supports:%s%s%s%s%s, enabled:%s%s%s%s%s",
		 this->features & PADLOCK_RNG_AVAILABLE ? " RNG" : "",
		 this->features & PADLOCK_ACE_AVAILABLE ? " ACE" : "",
		 this->features & PADLOCK_ACE2_AVAILABLE ? " ACE2" : "",
		 this->features & PADLOCK_PHE_AVAILABLE ? " PHE" : "",
		 this->features & PADLOCK_PMM_AVAILABLE ? " PMM" : "",
		 this->features & PADLOCK_RNG_ENABLED ? " RNG" : "",
		 this->features & PADLOCK_ACE_ENABLED ? " ACE" : "",
		 this->features & PADLOCK_ACE2_ENABLED ? " ACE2" : "",
		 this->features & PADLOCK_PHE_ENABLED ? " PHE" : "",
		 this->features & PADLOCK_PMM_ENABLED ? " PMM" : "");

	return &this->public.plugin;
}

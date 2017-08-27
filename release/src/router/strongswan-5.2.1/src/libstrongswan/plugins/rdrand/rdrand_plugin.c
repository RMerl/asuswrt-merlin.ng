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

/**
 * CPU feature flags, returned via cpuid(1)
 */
enum cpuid_feature_t {
	CPUID_RDRAND =		(1<<30),
};

/**
 * Get cpuid for info, return eax, ebx, ecx and edx.
 * -fPIC requires to save ebx on IA-32.
 */
static void cpuid(u_int op, u_int *a, u_int *b, u_int *c, u_int *d)
{
#ifdef __x86_64__
	asm("cpuid" : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d) : "a" (op));
#else /* __i386__ */
	asm("pushl %%ebx;"
		"cpuid;"
		"movl %%ebx, %1;"
		"popl %%ebx;"
		: "=a" (*a), "=r" (*b), "=c" (*c), "=d" (*d) : "a" (op));
#endif /* __x86_64__ / __i386__*/
}

/**
 * Check if we have RDRAND instruction
 */
static bool have_rdrand()
{
	char vendor[3 * sizeof(u_int32_t) + 1];
	u_int a, b, c, d;

	cpuid(0, &a, &b, &c, &d);
	/* VendorID string is in b-d-c (yes, in this order) */
	snprintf(vendor, sizeof(vendor), "%.4s%.4s%.4s", &b, &d, &c);

	/* check if we have an Intel CPU */
	if (streq(vendor, "GenuineIntel"))
	{
		cpuid(1, &a, &b, &c, &d);
		if (c & CPUID_RDRAND)
		{
			DBG2(DBG_LIB, "detected RDRAND support on %s CPU", vendor);
			return TRUE;
		}
	}
	DBG2(DBG_LIB, "no RDRAND support on %s CPU, disabled", vendor);
	return FALSE;
}

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
	if (have_rdrand())
	{
		return countof(f);
	}
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

/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2012-2014 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tests/test_runner.h>

#include <library.h>
#include <daemon.h>

#include "tkm.h"
#include "tkm_nonceg.h"
#include "tkm_diffie_hellman.h"
#include "tkm_kernel_ipsec.h"

/* declare test suite constructors */
#define TEST_SUITE(x) test_suite_t* x();
#define TEST_SUITE_DEPEND(x, ...) TEST_SUITE(x)
#include "tests.h"
#undef TEST_SUITE
#undef TEST_SUITE_DEPEND

static test_configuration_t tests[] = {
#define TEST_SUITE(x) \
	{ .suite = x, },
#define TEST_SUITE_DEPEND(x, type, ...) \
	{ .suite = x, .feature = PLUGIN_DEPENDS(type, __VA_ARGS__) },
#include "tests.h"
	{ .suite = NULL, }
};

static bool tkm_initialized = false;

static bool test_runner_init(bool init)
{
	bool result = TRUE;

	if (init)
	{
		libcharon_init();
		lib->settings->set_int(lib->settings,
							   "test-runner.filelog.stdout.default", 0);
		charon->load_loggers(charon);

		/* Register TKM specific plugins */
		static plugin_feature_t features[] = {
			PLUGIN_REGISTER(NONCE_GEN, tkm_nonceg_create),
				PLUGIN_PROVIDE(NONCE_GEN),
			PLUGIN_CALLBACK(kernel_ipsec_register, tkm_kernel_ipsec_create),
				PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
		};
		lib->plugins->add_static_features(lib->plugins, "tkm-tests", features,
										  countof(features), TRUE, NULL, NULL);

		lib->settings->set_int(lib->settings, "%s.dh_mapping.%d", 1,
							   lib->ns, MODP_3072_BIT);
		lib->settings->set_int(lib->settings, "%s.dh_mapping.%d", 2,
							   lib->ns, MODP_4096_BIT);
		register_dh_mapping();

		plugin_loader_add_plugindirs(BUILDDIR "/src/libstrongswan/plugins",
									 PLUGINS);
		plugin_loader_add_plugindirs(BUILDDIR "/src/libcharon/plugins",
									 PLUGINS);
		if (charon->initialize(charon, PLUGINS))
		{
			if (!tkm_initialized)
			{
				if (!tkm_init())
				{
					return FALSE;
				}
				tkm_initialized = true;
			}
			return TRUE;
		}
		result = FALSE;
	}

	destroy_dh_mapping();
	libcharon_deinit();
	return result;
}

int main(int argc, char *argv[])
{
	bool result;

	/* disable leak detective because of how tkm_init/deinit is called, which
	 * does not work otherwise due to limitations of the external libraries */
	setenv("LEAK_DETECTIVE_DISABLE", "1", 1);

	result = test_runner_run("tkm", tests, test_runner_init);
	tkm_deinit();

	return result;
}

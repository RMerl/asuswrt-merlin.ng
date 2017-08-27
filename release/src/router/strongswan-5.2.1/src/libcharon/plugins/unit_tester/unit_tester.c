/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#include "unit_tester.h"

#include <daemon.h>

typedef struct private_unit_tester_t private_unit_tester_t;
typedef struct unit_test_t unit_test_t;
typedef enum test_status_t test_status_t;

/**
 * private data of unit_tester
 */
struct private_unit_tester_t {

	/**
	 * public functions
	 */
	unit_tester_t public;
};

struct unit_test_t {

	/**
	 * name of the test
	 */
	char *name;

	/**
	 * test function
	 */
	bool (*test)(void);

	/**
	 * run the test?
	 */
	bool enabled;
};

#undef DEFINE_TEST
#define DEFINE_TEST(name, function, enabled) bool function();
#include <plugins/unit_tester/tests.h>
#undef DEFINE_TEST
#define DEFINE_TEST(name, function, enabled) {name, function, enabled},
static unit_test_t tests[] = {
#include <plugins/unit_tester/tests.h>
};

static void run_tests(private_unit_tester_t *this)
{
	int i, run = 0, failed = 0, success = 0, skipped = 0;

	DBG1(DBG_CFG, "running unit tests, %d tests registered",
		 sizeof(tests)/sizeof(unit_test_t));

	for (i = 0; i < sizeof(tests)/sizeof(unit_test_t); i++)
	{
		if (tests[i].enabled)
		{
			run++;
			if (tests[i].test())
			{
				DBG1(DBG_CFG, "test '%s' successful", tests[i].name);
				success++;
			}
			else
			{
				DBG1(DBG_CFG, "test '%s' failed", tests[i].name);
				failed++;
			}
		}
		else
		{
			DBG1(DBG_CFG, "test '%s' disabled", tests[i].name);
			skipped++;
		}
	}
	DBG1(DBG_CFG, "%d/%d tests successful (%d failed, %d disabled)",
		 success, run, failed, skipped);
}

METHOD(plugin_t, get_name, char*,
	private_unit_tester_t *this)
{
	return "unit-tester";
}

/**
 * We currently don't depend explicitly on any plugin features.  But in case
 * activated tests depend on such features we at least try to run them in plugin
 * order.
 */
static bool plugin_cb(private_unit_tester_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		run_tests(this);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_unit_tester_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "unit-tester"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_unit_tester_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *unit_tester_plugin_create()
{
	private_unit_tester_t *this;

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

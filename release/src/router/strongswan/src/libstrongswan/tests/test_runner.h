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

/**
 * @defgroup libtest libtest
 *
 * @defgroup test_utils test_utils
 * @ingroup libtest
 *
 * @defgroup test_runner test_runner
 * @{ @ingroup libtest
 */

#ifndef TEST_RUNNER_H_
#define TEST_RUNNER_H_

#include "test_suite.h"

#include <plugins/plugin_feature.h>

typedef struct test_configuration_t test_configuration_t;

/**
 * Callback called before and after each test case to de-/initialize the
 * environment (e.g. to load plugins).  It is also called before and after the
 * test suites are loaded.
 *
 * It is called after libstrongswan has been initialized and likewise before it
 * gets deinitialized.
 *
 * @param init			TRUE during initialization
 * @return				FALSE if de-/init failed
 */
typedef bool (*test_runner_init_t)(bool init);

/**
 * Test configuration, suite constructor with plugin dependency
 */
struct test_configuration_t {

	/**
	 * Constructor function to create suite.
	 */
	test_suite_t *(*suite)();

	/**
	 * Plugin feature this test suite depends on
	 */
	plugin_feature_t feature;
};

/**
 * Run test configuration.
 *
 * The configs array must be terminated with a NULL element. The following
 * environment variables are currently supported:
 *
 * - TESTS_VERBOSITY: Numerical loglevel for debug log
 * - TESTS_STRONGSWAN_CONF: Specify a path to a custom strongswan.conf
 * - TESTS_PLUGINS: Specify an explicit list of plugins to load
 * - TESTS_RUNNERS: Run specific test runners only
 * - TESTS_SUITES: Run specific test suites only
 * - TESTS_SUITES_EXCLUDE: Don't run specific test suites
 * - TESTS_REDUCED_KEYLENGTHS: Test minimal keylengths for public key tests only
 *
 * Please note that TESTS_PLUGINS actually must be implemented by the init
 * callback function, as plugin loading is delegated.
 *
 * EXIT_SUCCESS is returned right away if TESTS_RUNNERS is defined but the name
 * passed to this function is not contained in it.
 *
 * @param name			name of test runner
 * @param config		test suite constructors with dependencies
 * @param init_cb		init/deinit callback
 * @return				test result, EXIT_SUCCESS if all tests passed
 */
int test_runner_run(const char *name, test_configuration_t config[],
					test_runner_init_t init_cb);

#endif /** TEST_RUNNER_H_ @}*/

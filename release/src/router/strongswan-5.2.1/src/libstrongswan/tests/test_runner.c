/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include "test_runner.h"

#include <library.h>
#include <threading/thread.h>
#include <plugins/plugin_feature.h>
#include <collections/array.h>
#include <utils/test.h>

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

/**
 * Get a tty color escape character for stderr
 */
#define TTY(color) tty_escape_get(2, TTY_FG_##color)

/**
 * A global symbol indicating libtest linkage
 */
#ifdef WIN32
__declspec(dllexport)
#endif
bool test_runner_available = TRUE;

/**
 * Destroy a single test suite and associated data
 */
static void destroy_suite(test_suite_t *suite)
{
	test_case_t *tcase;

	while (array_remove(suite->tcases, 0, &tcase))
	{
		array_destroy(tcase->functions);
		array_destroy(tcase->fixtures);
	}
	free(suite);
}

/**
 * Filter loaded test suites, either remove suites listed (exclude=TRUE), or all
 * that are not listed (exclude=FALSE).
 */
static void apply_filter(array_t *loaded, char *filter, bool exclude)
{
	enumerator_t *enumerator, *names;
	hashtable_t *listed;
	test_suite_t *suite;
	char *name;

	listed = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8);
	names = enumerator_create_token(filter, ",", " ");
	while (names->enumerate(names, &name))
	{
		listed->put(listed, name, name);
	}
	enumerator = array_create_enumerator(loaded);
	while (enumerator->enumerate(enumerator, &suite))
	{
		if ((exclude && listed->get(listed, suite->name)) ||
			(!exclude && !listed->get(listed, suite->name)))
		{
			array_remove_at(loaded, enumerator);
			destroy_suite(suite);
		}
	}
	enumerator->destroy(enumerator);
	listed->destroy(listed);
	names->destroy(names);
}

/**
 * Removes and destroys test suites that are not selected or
 * explicitly excluded.
 */
static void filter_suites(array_t *loaded)
{
	char *filter;

	filter = getenv("TESTS_SUITES");
	if (filter)
	{
		apply_filter(loaded, filter, FALSE);
	}
	filter = getenv("TESTS_SUITES_EXCLUDE");
	if (filter)
	{
		apply_filter(loaded, filter, TRUE);
	}
}

/**
 * Load all available test suites, or optionally only selected ones.
 */
static array_t *load_suites(test_configuration_t configs[],
							test_runner_init_t init, char *cfg)
{
	array_t *suites;
	bool old = FALSE;
	int i;

	library_init(cfg, "test-runner");

	test_setup_handler();

	if (init && !init(TRUE))
	{
		library_deinit();
		return NULL;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}

	suites = array_create(0, 0);

	for (i = 0; configs[i].suite; i++)
	{
		if (configs[i].feature.type == 0 ||
			lib->plugins->has_feature(lib->plugins, configs[i].feature))
		{
			array_insert(suites, -1, configs[i].suite());
		}
	}
	filter_suites(suites);

	if (lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}

	if (init)
	{
		init(FALSE);
	}
	library_deinit();

	return suites;
}

/**
 * Unload and destroy test suites and associated data
 */
static void unload_suites(array_t *suites)
{
	test_suite_t *suite;

	while (array_remove(suites, 0, &suite))
	{
		destroy_suite(suite);
	}
	array_destroy(suites);
}

/**
 * Run a single test function, return FALSE on failure
 */
static bool run_test(test_function_t *tfun, int i)
{
	if (test_restore_point())
	{
		tfun->cb(i);
		return TRUE;
	}
	return FALSE;
}

/**
 * Invoke fixture setup/teardown
 */
static bool call_fixture(test_case_t *tcase, bool up)
{
	enumerator_t *enumerator;
	test_fixture_t *fixture;
	bool failure = FALSE;

	enumerator = array_create_enumerator(tcase->fixtures);
	while (enumerator->enumerate(enumerator, &fixture))
	{
		if (test_restore_point())
		{
			if (up)
			{
				if (fixture->setup)
				{
					fixture->setup();
				}
			}
			else
			{
				if (fixture->teardown)
				{
					fixture->teardown();
				}
			}
		}
		else
		{
			failure = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return !failure;
}

/**
 * Test initialization, initializes libstrongswan for the next run
 */
static bool pre_test(test_runner_init_t init, char *cfg)
{
	level_t level = LEVEL_SILENT;
	char *verbosity;

	library_init(cfg, "test-runner");

	/* use non-blocking RNG to generate keys fast */
	lib->settings->set_default_str(lib->settings,
			"libstrongswan.plugins.random.random",
			lib->settings->get_str(lib->settings,
				"libstrongswan.plugins.random.urandom", "/dev/urandom"));
	/* same for the gcrypt plugin */
	lib->settings->set_default_str(lib->settings,
			"libstrongswan.plugins.gcrypt.quick_random", "yes");

	if (lib->leak_detective)
	{
		/* disable leak reports during testing */
		lib->leak_detective->set_report_cb(lib->leak_detective,
										   NULL, NULL, NULL);
	}
	if (init && !init(TRUE))
	{
		library_deinit();
		return FALSE;
	}
	verbosity = getenv("TESTS_VERBOSITY");
	if (verbosity)
	{
		level = atoi(verbosity);
	}
	dbg_default_set_level(level);
	return TRUE;
}

/**
 * Failure description
 */
typedef struct {
	char *name;
	char msg[512 - sizeof(char*) - 2 * sizeof(int)];
	const char *file;
	int line;
	int i;
	backtrace_t *bt;
} failure_t;

/**
 * Data passed to leak report callbacks
 */
typedef struct {
	array_t *failures;
	char *name;
	int i;
	int leaks;
} report_data_t;

/**
 * Leak report callback, build failures from leaks
 */
static void report_leaks(report_data_t *data, int count, size_t bytes,
						 backtrace_t *bt, bool detailed)
{
	failure_t failure = {
		.name = data->name,
		.i = data->i,
		.bt = bt->clone(bt),
	};

	snprintf(failure.msg, sizeof(failure.msg),
			 "Leak detected: %d allocations using %zu bytes", count, bytes);

	array_insert(data->failures, -1, &failure);
}

/**
 * Leak summary callback, check if any leaks found
 */
static void sum_leaks(report_data_t *data, int count, size_t bytes,
					  int whitelisted)
{
	data->leaks = count;
}

/**
 * Do library cleanup and optionally check for memory leaks
 */
static bool post_test(test_runner_init_t init, bool check_leaks,
					  array_t *failures, char *name, int i, int *leaks)
{
	report_data_t data = {
		.failures = failures,
		.name = name,
		.i = i,
	};

	if (init)
	{
		if (test_restore_point())
		{
			init(FALSE);
		}
		else
		{
			library_deinit();
			return FALSE;
		}
	}
	if (check_leaks && lib->leak_detective)
	{
		lib->leak_detective->set_report_cb(lib->leak_detective,
								(leak_detective_report_cb_t)report_leaks,
								(leak_detective_summary_cb_t)sum_leaks, &data);
	}
	library_deinit();

	*leaks = data.leaks;
	return TRUE;
}

/**
 * Collect failure information, add failure_t to array
 */
static void collect_failure_info(array_t *failures, char *name, int i)
{
	failure_t failure = {
		.name = name,
		.i = i,
		.bt = test_failure_backtrace(),
	};

	failure.line = test_failure_get(failure.msg, sizeof(failure.msg),
									&failure.file);

	array_insert(failures, -1, &failure);
}

/**
 * Print array of collected failure_t to stderr
 */
static void print_failures(array_t *failures)
{
	failure_t failure;

	threads_init();
	backtrace_init();

	while (array_remove(failures, 0, &failure))
	{
		fprintf(stderr, "      %sFailure in '%s': %s (",
				TTY(RED), failure.name, failure.msg);
		if (failure.line)
		{
			fprintf(stderr, "%s:%d, ", failure.file, failure.line);
		}
		fprintf(stderr, "i = %d)%s\n", failure.i, TTY(DEF));
		if (failure.bt)
		{
			failure.bt->log(failure.bt, stderr, TRUE);
			failure.bt->destroy(failure.bt);
		}
	}

	backtrace_deinit();
	threads_deinit();
}

/**
 * Run a single test case with fixtures
 */
static bool run_case(test_case_t *tcase, test_runner_init_t init, char *cfg)
{
	enumerator_t *enumerator;
	test_function_t *tfun;
	int passed = 0;
	array_t *failures;

	failures = array_create(sizeof(failure_t), 0);

	fprintf(stderr, "    Running case '%s': ", tcase->name);
	fflush(stderr);

	enumerator = array_create_enumerator(tcase->functions);
	while (enumerator->enumerate(enumerator, &tfun))
	{
		int i, rounds = 0;

		for (i = tfun->start; i < tfun->end; i++)
		{
			if (pre_test(init, cfg))
			{
				bool ok = FALSE;
				int leaks = 0;

				test_setup_timeout(tcase->timeout);

				if (call_fixture(tcase, TRUE))
				{
					if (run_test(tfun, i))
					{
						if (call_fixture(tcase, FALSE))
						{
							ok = TRUE;
						}
					}
					else
					{
						call_fixture(tcase, FALSE);
					}
				}
				if (!post_test(init, ok, failures, tfun->name, i, &leaks))
				{
					ok = FALSE;
				}

				test_setup_timeout(0);

				if (ok)
				{
					if (!leaks)
					{
						rounds++;
						fprintf(stderr, "%s+%s", TTY(GREEN), TTY(DEF));
					}
				}
				else
				{
					collect_failure_info(failures, tfun->name, i);
				}
				if (!ok || leaks)
				{
					fprintf(stderr, "%s-%s", TTY(RED), TTY(DEF));
				}
			}
			else
			{
				fprintf(stderr, "!");
			}
		}
		fflush(stderr);
		if (rounds == tfun->end - tfun->start)
		{
			passed++;
		}
	}
	enumerator->destroy(enumerator);

	fprintf(stderr, "\n");

	print_failures(failures);
	array_destroy(failures);

	return passed == array_count(tcase->functions);
}

/**
 * Run a single test suite
 */
static bool run_suite(test_suite_t *suite, test_runner_init_t init, char *cfg)
{
	enumerator_t *enumerator;
	test_case_t *tcase;
	int passed = 0;

	fprintf(stderr, "  Running suite '%s':\n", suite->name);

	enumerator = array_create_enumerator(suite->tcases);
	while (enumerator->enumerate(enumerator, &tcase))
	{
		if (run_case(tcase, init, cfg))
		{
			passed++;
		}
	}
	enumerator->destroy(enumerator);

	if (passed == array_count(suite->tcases))
	{
		fprintf(stderr, "  %sPassed all %u '%s' test cases%s\n",
				TTY(GREEN), array_count(suite->tcases), suite->name, TTY(DEF));
		return TRUE;
	}
	fprintf(stderr, "  %sPassed %u/%u '%s' test cases%s\n",
			TTY(RED), passed, array_count(suite->tcases), suite->name, TTY(DEF));
	return FALSE;
}

/**
 * See header.
 */
int test_runner_run(const char *name, test_configuration_t configs[],
					test_runner_init_t init)
{
	array_t *suites;
	test_suite_t *suite;
	enumerator_t *enumerator;
	int passed = 0, result;
	char *cfg;

	/* redirect all output to stderr (to redirect make's stdout to /dev/null) */
	dup2(2, 1);

	cfg = getenv("TESTS_STRONGSWAN_CONF");

	suites = load_suites(configs, init, cfg);
	if (!suites)
	{
		return EXIT_FAILURE;
	}

	fprintf(stderr, "Running %u '%s' test suites:\n", array_count(suites), name);

	enumerator = array_create_enumerator(suites);
	while (enumerator->enumerate(enumerator, &suite))
	{
		if (run_suite(suite, init, cfg))
		{
			passed++;
		}
	}
	enumerator->destroy(enumerator);

	if (passed == array_count(suites))
	{
		fprintf(stderr, "%sPassed all %u '%s' suites%s\n",
				TTY(GREEN), array_count(suites), name, TTY(DEF));
		result = EXIT_SUCCESS;
	}
	else
	{
		fprintf(stderr, "%sPassed %u of %u '%s' suites%s\n",
				TTY(RED), passed, array_count(suites), name, TTY(DEF));
		result = EXIT_FAILURE;
	}

	unload_suites(suites);

	return result;
}

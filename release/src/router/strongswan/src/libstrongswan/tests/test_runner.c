/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2013 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
#include <time.h>

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
 * Destroy data associated with a test case.
 */
static void destroy_case(test_case_t *tcase)
{
	array_destroy(tcase->functions);
	array_destroy(tcase->fixtures);
	free(tcase);
}

/**
 * Destroy a single test suite and associated data.
 */
static void destroy_suite(test_suite_t *suite)
{
	array_destroy_function(suite->tcases, (void*)destroy_case, NULL);
	free(suite);
}

/**
 * Identifies on which component to apply the given filter.
 */
typedef enum {
	FILTER_SUITES,
	FILTER_CASES,
	FILTER_FUNCTIONS,
} filter_component_t;

/**
 * Check if the component with the given name should be filtered/removed.
 */
static bool filter_name(const char *name, hashtable_t *names, bool exclude)
{
	return (exclude && names->get(names, name)) ||
		   (!exclude && !names->get(names, name));
}

/**
 * Filter loaded test suites/cases/functions, either remove components listed
 * (exclude=TRUE), or all that are not listed (exclude=FALSE).
 * Empty test cases/suites are removed and destroyed.
 */
static void apply_filter(array_t *loaded, filter_component_t comp, char *filter,
						 bool exclude)
{
	enumerator_t *enumerator, *tcases, *functions, *names;
	hashtable_t *listed;
	test_suite_t *suite;
	test_case_t *tcase;
	test_function_t *func;
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
		if (comp == FILTER_SUITES)
		{
			if (filter_name(suite->name, listed, exclude))
			{
				array_remove_at(loaded, enumerator);
				destroy_suite(suite);
			}
			continue;
		}
		tcases = array_create_enumerator(suite->tcases);
		while (tcases->enumerate(tcases, &tcase))
		{
			if (comp == FILTER_CASES)
			{
				if (filter_name(tcase->name, listed, exclude))
				{
					array_remove_at(suite->tcases, tcases);
					destroy_case(tcase);
				}
				continue;
			}
			functions = array_create_enumerator(tcase->functions);
			while (functions->enumerate(functions, &func))
			{
				if (filter_name(func->name, listed, exclude))
				{
					array_remove_at(tcase->functions, functions);
				}
			}
			functions->destroy(functions);

			if (!array_count(tcase->functions))
			{
				array_remove_at(suite->tcases, tcases);
				destroy_case(tcase);
			}
		}
		tcases->destroy(tcases);

		if (!array_count(suite->tcases))
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
 * Check if the given string is contained in the filter string.
 */
static bool is_in_filter(const char *find, char *filter)
{
	enumerator_t *names;
	bool found = FALSE;
	char *name;

	names = enumerator_create_token(filter, ",", " ");
	while (names->enumerate(names, &name))
	{
		if (streq(name, find))
		{
			found = TRUE;
			break;
		}
	}
	names->destroy(names);
	return found;
}

/**
 * Removes and destroys test suites/cases/functions that are not selected or
 * explicitly excluded. Takes names of two environment variables.
 */
static void filter_components(array_t *loaded, filter_component_t comp,
							  char *sel, char *exc)
{
	char *filter;

	filter = getenv(sel);
	if (filter)
	{
		apply_filter(loaded, comp, filter, FALSE);
	}
	filter = getenv(exc);
	if (filter)
	{
		apply_filter(loaded, comp, filter, TRUE);
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
	filter_components(suites, FILTER_SUITES, "TESTS_SUITES",
					  "TESTS_SUITES_EXCLUDE");
	filter_components(suites, FILTER_CASES, "TESTS_CASES",
					  "TESTS_CASES_EXCLUDE");
	filter_components(suites, FILTER_FUNCTIONS, "TESTS_FUNCTIONS",
					  "TESTS_FUNCTIONS_EXCLUDE");

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
	thread_cleanup_popall();
	return FALSE;
}

/**
 * Invoke fixture setup/teardown
 */
static bool call_fixture(test_case_t *tcase, bool up, int i)
{
	enumerator_t *enumerator;
	test_fixture_t *fixture;
	volatile bool failure = FALSE;

	enumerator = array_create_enumerator(tcase->fixtures);
	while (enumerator->enumerate(enumerator, &fixture))
	{
		if (test_restore_point())
		{
			if (up)
			{
				if (fixture->setup)
				{
					fixture->setup(i);
				}
			}
			else
			{
				if (fixture->teardown)
				{
					fixture->teardown(i);
				}
			}
		}
		else
		{
			thread_cleanup_popall();
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
	return TRUE;
}

/**
 * Failure description
 */
typedef struct {
	char *name;
	char msg[4096 - sizeof(char*) - 2 * sizeof(int)];
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
			thread_cleanup_popall();
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
 * Context data to collect warnings
 */
typedef struct {
	char *name;
	int i;
	array_t *warnings;
} warning_ctx_t;

/**
 * Callback to collect warnings
 */
CALLBACK(warning_cb, void,
	warning_ctx_t *ctx, const char *msg, const char *file, const int line)
{
	failure_t warning = {
		.name = ctx->name,
		.i = ctx->i,
		.file = file,
		.line = line,
	};

	strncpy(warning.msg, msg, sizeof(warning.msg) - 1);
	warning.msg[sizeof(warning.msg)-1] = 0;
	array_insert(ctx->warnings, -1, &warning);
}

/**
 * Collect warning information, add failure_t to array
 */
static bool collect_warning_info(array_t *warnings, char *name, int i)
{
	warning_ctx_t ctx = {
		.name = name,
		.i = i,
		.warnings = warnings,
	};

	return test_warnings_get(warning_cb, &ctx);
}

/**
 * Print array of collected failure_t to stderr
 */
static void print_failures(array_t *failures, bool warnings)
{
	failure_t failure;

	threads_init();
	backtrace_init();

	while (array_remove(failures, 0, &failure))
	{
		if (warnings)
		{
			fprintf(stderr, "      %sWarning in '%s': %s (",
					TTY(YELLOW), failure.name, failure.msg);
		}
		else
		{
			fprintf(stderr, "      %sFailure in '%s': %s (",
					TTY(RED), failure.name, failure.msg);
		}
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

#if defined(CLOCK_THREAD_CPUTIME_ID) && defined(HAVE_CLOCK_GETTIME)

/**
 * Start a timer
 */
static void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

/**
 * End a timer, return ms
 */
static double end_timing(struct timespec *start)
{
	struct timespec end;

	if (!getenv("TESTS_TIMING"))
	{
		return 0;
	}
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) / 1000000.0 +
			(end.tv_sec - start->tv_sec) * 1000.0;
}

#else /* CLOCK_THREAD_CPUTIME_ID */

#define start_timing(start) ((start)->tv_sec = 0, (start)->tv_nsec = 0)
#define end_timing(...) (0)

#endif /* CLOCK_THREAD_CPUTIME_ID */

/**
 * Determine the configured iterations to run
 */
static hashtable_t *get_iterations()
{
	enumerator_t *enumerator;
	hashtable_t *config = NULL;
	char *iterations, *iter;

	iterations = getenv("TESTS_ITERATIONS");
	if (!iterations)
	{
		return NULL;
	}

	config = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 8);
	enumerator = enumerator_create_token(iterations, ",", " ");
	while (enumerator->enumerate(enumerator, &iter))
	{
		/* add 1 so we can store 0 */
		config->put(config, (void*)(uintptr_t)atoi(iter)+1, config);
	}
	enumerator->destroy(enumerator);
	if (!config->get_count(config))
	{
		config->destroy(config);
		config = NULL;
	}
	return config;
}

/**
 * Run a single test case with fixtures
 */
static bool run_case(test_case_t *tcase, test_runner_init_t init, char *cfg,
					 level_t level)
{
	enumerator_t *enumerator;
	test_function_t *tfun;
	hashtable_t *iterations;
	double *times;
	double total_time = 0;
	int tests = 0, ti = 0, passed = 0;
	array_t *failures, *warnings;

	/* determine the number of tests we will run */
	enumerator = array_create_enumerator(tcase->functions);
	while (enumerator->enumerate(enumerator, &tfun))
	{
		tests += tfun->end - tfun->start;
	}
	enumerator->destroy(enumerator);
	times = calloc(tests, sizeof(double));

	failures = array_create(sizeof(failure_t), 0);
	warnings = array_create(sizeof(failure_t), 0);

	fprintf(stderr, "    Running case '%s': ", tcase->name);
	fflush(stderr);

	iterations = get_iterations();

	enumerator = array_create_enumerator(tcase->functions);
	while (enumerator->enumerate(enumerator, &tfun))
	{
		int i, rounds = 0, skipped = 0;

		for (i = tfun->start; i < tfun->end; i++)
		{
			if (iterations && !iterations->get(iterations, (void*)(uintptr_t)i+1))
			{
				skipped++;
				continue;
			}
			if (pre_test(init, cfg))
			{
				struct timespec start;
				bool ok = FALSE;
				int leaks = 0;

				if (level >= 0)
				{
					fprintf(stderr, "\nRunning function '%s' [%d]:\n",
							tfun->name, i);
				}

				test_setup_timeout(tcase->timeout);
				start_timing(&start);

				if (call_fixture(tcase, TRUE, i))
				{
					if (run_test(tfun, i))
					{
						if (call_fixture(tcase, FALSE, i))
						{
							ok = TRUE;
						}
					}
					else
					{
						call_fixture(tcase, FALSE, i);
					}
				}
				if (!post_test(init, ok, failures, tfun->name, i, &leaks))
				{
					ok = FALSE;
				}

				times[ti] = end_timing(&start);
				total_time += times[ti++];
				test_setup_timeout(0);

				if (ok)
				{
					if (!leaks)
					{
						rounds++;
						if (!collect_warning_info(warnings, tfun->name, i))
						{
							fprintf(stderr, "%s+%s", TTY(GREEN), TTY(DEF));
						}
						else
						{
							fprintf(stderr, "%s~%s", TTY(YELLOW), TTY(DEF));
						}
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
		if (rounds == tfun->end - tfun->start - skipped)
		{
			passed++;
		}
	}
	enumerator->destroy(enumerator);

	if (total_time)
	{
		fprintf(stderr, " %s%s%.3f ms%s", tty_escape_get(2, TTY_BOLD),
				TTY(BLUE), total_time, tty_escape_get(2, TTY_RESET));
		if (ti > 1)
		{
			fprintf(stderr, " %s[", TTY(BLUE));
			for (ti = 0; ti < tests; ti++)
			{
				fprintf(stderr, "%s%.3f ms", times[ti], ti == 0 ? "" : ", ");
			}
			fprintf(stderr, "]%s", TTY(DEF));
		}
	}
	fprintf(stderr, "\n");

	print_failures(warnings, TRUE);
	print_failures(failures, FALSE);
	array_destroy(failures);
	array_destroy(warnings);
	DESTROY_IF(iterations);
	free(times);

	return passed == array_count(tcase->functions);
}

/**
 * Run a single test suite
 */
static bool run_suite(test_suite_t *suite, test_runner_init_t init, char *cfg,
					  level_t level)
{
	enumerator_t *enumerator;
	test_case_t *tcase;
	int passed = 0;

	fprintf(stderr, "  Running suite '%s':\n", suite->name);

	enumerator = array_create_enumerator(suite->tcases);
	while (enumerator->enumerate(enumerator, &tcase))
	{
		if (run_case(tcase, init, cfg, level))
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
 * Configure log levels for specific groups
 */
static void setup_log_levels(level_t *base)
{
	char buf[BUF_LEN], *verbosity;
	debug_t group;
	level_t level;

	for (group = 0; group < DBG_MAX; group++)
	{
		snprintf(buf, sizeof(buf), "TESTS_VERBOSITY_%s",
				 enum_to_name(debug_names, group));

		verbosity = getenv(buf);
		if (verbosity)
		{
			level = atoi(verbosity);
			dbg_default_set_level_group(group, level);
			*base = max(*base, level);
		}
	}
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
	level_t level = LEVEL_SILENT;
	char *cfg, *runners, *verbosity;

	/* redirect all output to stderr (to redirect make's stdout to /dev/null) */
	dup2(2, 1);

	runners = getenv("TESTS_RUNNERS");
	if (runners && !is_in_filter(name, runners))
	{
		return EXIT_SUCCESS;
	}

	cfg = getenv("TESTS_STRONGSWAN_CONF");

	suites = load_suites(configs, init, cfg);
	if (!suites)
	{
		return EXIT_FAILURE;
	}

	verbosity = getenv("TESTS_VERBOSITY");
	if (verbosity)
	{
		level = atoi(verbosity);
	}
	dbg_default_set_level(level);

	setup_log_levels(&level);

	fprintf(stderr, "Running %u '%s' test suites:\n", array_count(suites), name);

	enumerator = array_create_enumerator(suites);
	while (enumerator->enumerate(enumerator, &suite))
	{
		if (run_suite(suite, init, cfg, level))
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

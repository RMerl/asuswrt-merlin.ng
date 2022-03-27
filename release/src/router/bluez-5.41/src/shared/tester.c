/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <glib.h>

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#endif

#include "src/shared/util.h"
#include "src/shared/tester.h"

#define COLOR_OFF	"\x1B[0m"
#define COLOR_BLACK	"\x1B[0;30m"
#define COLOR_RED	"\x1B[0;31m"
#define COLOR_GREEN	"\x1B[0;32m"
#define COLOR_YELLOW	"\x1B[0;33m"
#define COLOR_BLUE	"\x1B[0;34m"
#define COLOR_MAGENTA	"\x1B[0;35m"
#define COLOR_CYAN	"\x1B[0;36m"
#define COLOR_WHITE	"\x1B[0;37m"
#define COLOR_HIGHLIGHT	"\x1B[1;39m"

#define print_text(color, fmt, args...) \
		printf(color fmt COLOR_OFF "\n", ## args)

#define print_summary(label, color, value, fmt, args...) \
			printf("%-52s " color "%-10s" COLOR_OFF fmt "\n", \
							label, value, ## args)

#define print_progress(name, color, fmt, args...) \
		printf(COLOR_HIGHLIGHT "%s" COLOR_OFF " - " \
				color fmt COLOR_OFF "\n", name, ## args)

enum test_result {
	TEST_RESULT_NOT_RUN,
	TEST_RESULT_PASSED,
	TEST_RESULT_FAILED,
	TEST_RESULT_TIMED_OUT,
};

enum test_stage {
	TEST_STAGE_INVALID,
	TEST_STAGE_PRE_SETUP,
	TEST_STAGE_SETUP,
	TEST_STAGE_RUN,
	TEST_STAGE_TEARDOWN,
	TEST_STAGE_POST_TEARDOWN,
};

struct test_case {
	char *name;
	enum test_result result;
	enum test_stage stage;
	const void *test_data;
	tester_data_func_t pre_setup_func;
	tester_data_func_t setup_func;
	tester_data_func_t test_func;
	tester_data_func_t teardown_func;
	tester_data_func_t post_teardown_func;
	gdouble start_time;
	gdouble end_time;
	unsigned int timeout;
	unsigned int timeout_id;
	unsigned int teardown_id;
	tester_destroy_func_t destroy;
	void *user_data;
};

static GMainLoop *main_loop;

static GList *test_list;
static GList *test_current;
static GTimer *test_timer;

static gboolean option_version = FALSE;
static gboolean option_quiet = FALSE;
static gboolean option_debug = FALSE;
static gboolean option_list = FALSE;
static const char *option_prefix = NULL;

static void test_destroy(gpointer data)
{
	struct test_case *test = data;

	if (test->timeout_id > 0)
		g_source_remove(test->timeout_id);

	if (test->teardown_id > 0)
		g_source_remove(test->teardown_id);

	if (test->destroy)
		test->destroy(test->user_data);

	free(test->name);
	free(test);
}

void tester_print(const char *format, ...)
{
	va_list ap;

	if (tester_use_quiet())
		return;

	printf("  %s", COLOR_WHITE);
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("%s\n", COLOR_OFF);
}

void tester_debug(const char *format, ...)
{
	va_list ap;

	if (!tester_use_debug())
		return;

	printf("  %s", COLOR_WHITE);
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("%s\n", COLOR_OFF);
}

void tester_warn(const char *format, ...)
{
	va_list ap;

	printf("  %s", COLOR_WHITE);
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("%s\n", COLOR_OFF);
}

static void default_pre_setup(const void *test_data)
{
	tester_pre_setup_complete();
}

static void default_setup(const void *test_data)
{
	tester_setup_complete();
}

static void default_teardown(const void *test_data)
{
	tester_teardown_complete();
}

static void default_post_teardown(const void *test_data)
{
	tester_post_teardown_complete();
}

void tester_add_full(const char *name, const void *test_data,
				tester_data_func_t pre_setup_func,
				tester_data_func_t setup_func,
				tester_data_func_t test_func,
				tester_data_func_t teardown_func,
				tester_data_func_t post_teardown_func,
				unsigned int timeout,
				void *user_data, tester_destroy_func_t destroy)
{
	struct test_case *test;

	if (!test_func)
		return;

	if (option_prefix && !g_str_has_prefix(name, option_prefix)) {
		if (destroy)
			destroy(user_data);
		return;
	}

	if (option_list) {
		printf("%s\n", name);
		if (destroy)
			destroy(user_data);
		return;
	}

	test = new0(struct test_case, 1);
	test->name = strdup(name);
	test->result = TEST_RESULT_NOT_RUN;
	test->stage = TEST_STAGE_INVALID;

	test->test_data = test_data;

	if (pre_setup_func)
		test->pre_setup_func = pre_setup_func;
	else
		test->pre_setup_func = default_pre_setup;

	if (setup_func)
		test->setup_func = setup_func;
	else
		test->setup_func = default_setup;

	test->test_func = test_func;

	if (teardown_func)
		test->teardown_func = teardown_func;
	else
		test->teardown_func = default_teardown;

	if (post_teardown_func)
		test->post_teardown_func = post_teardown_func;
	else
		test->post_teardown_func = default_post_teardown;

	test->timeout = timeout;

	test->destroy = destroy;
	test->user_data = user_data;

	test_list = g_list_append(test_list, test);
}

void tester_add(const char *name, const void *test_data,
					tester_data_func_t setup_func,
					tester_data_func_t test_func,
					tester_data_func_t teardown_func)
{
	tester_add_full(name, test_data, NULL, setup_func, test_func,
					teardown_func, NULL, 0, NULL, NULL);
}

void *tester_get_data(void)
{
	struct test_case *test;

	if (!test_current)
		return NULL;

	test = test_current->data;

	return test->user_data;
}

static int tester_summarize(void)
{
	unsigned int not_run = 0, passed = 0, failed = 0;
	gdouble execution_time;
	GList *list;

	printf("\n");
	print_text(COLOR_HIGHLIGHT, "");
	print_text(COLOR_HIGHLIGHT, "Test Summary");
	print_text(COLOR_HIGHLIGHT, "------------");

	for (list = g_list_first(test_list); list; list = g_list_next(list)) {
		struct test_case *test = list->data;
		gdouble exec_time;

		exec_time = test->end_time - test->start_time;

		switch (test->result) {
		case TEST_RESULT_NOT_RUN:
			print_summary(test->name, COLOR_YELLOW, "Not Run", "");
			not_run++;
			break;
		case TEST_RESULT_PASSED:
			print_summary(test->name, COLOR_GREEN, "Passed",
						"%8.3f seconds", exec_time);
			passed++;
			break;
		case TEST_RESULT_FAILED:
			print_summary(test->name, COLOR_RED, "Failed",
						"%8.3f seconds", exec_time);
			failed++;
			break;
		case TEST_RESULT_TIMED_OUT:
			print_summary(test->name, COLOR_RED, "Timed out",
						"%8.3f seconds", exec_time);
			failed++;
			break;
		}
        }

	printf("\nTotal: %d, "
		COLOR_GREEN "Passed: %d (%.1f%%)" COLOR_OFF ", "
		COLOR_RED "Failed: %d" COLOR_OFF ", "
		COLOR_YELLOW "Not Run: %d" COLOR_OFF "\n",
			not_run + passed + failed, passed,
			(not_run + passed + failed) ?
			(float) passed * 100 / (not_run + passed + failed) : 0,
			failed, not_run);

	execution_time = g_timer_elapsed(test_timer, NULL);
	printf("Overall execution time: %.3g seconds\n", execution_time);

	return failed;
}

static gboolean teardown_callback(gpointer user_data)
{
	struct test_case *test = user_data;

	test->teardown_id = 0;
	test->stage = TEST_STAGE_TEARDOWN;

	print_progress(test->name, COLOR_MAGENTA, "teardown");
	test->teardown_func(test->test_data);

#ifdef HAVE_VALGRIND_MEMCHECK_H
	VALGRIND_DO_ADDED_LEAK_CHECK;
#endif

	return FALSE;
}

static gboolean test_timeout(gpointer user_data)
{
	struct test_case *test = user_data;

	test->timeout_id = 0;

	if (!test_current)
		return FALSE;

	test->result = TEST_RESULT_TIMED_OUT;
	print_progress(test->name, COLOR_RED, "test timed out");

	g_idle_add(teardown_callback, test);

	return FALSE;
}

static void next_test_case(void)
{
	struct test_case *test;

	if (test_current)
		test_current = g_list_next(test_current);
	else
		test_current = test_list;

	if (!test_current) {
		g_timer_stop(test_timer);

		g_main_loop_quit(main_loop);
		return;
	}

	test = test_current->data;

	printf("\n");
	print_progress(test->name, COLOR_BLACK, "init");

	test->start_time = g_timer_elapsed(test_timer, NULL);

	if (test->timeout > 0)
		test->timeout_id = g_timeout_add_seconds(test->timeout,
							test_timeout, test);

	test->stage = TEST_STAGE_PRE_SETUP;

	test->pre_setup_func(test->test_data);
}

static gboolean setup_callback(gpointer user_data)
{
	struct test_case *test = user_data;

	test->stage = TEST_STAGE_SETUP;

	print_progress(test->name, COLOR_BLUE, "setup");
	test->setup_func(test->test_data);

	return FALSE;
}

static gboolean run_callback(gpointer user_data)
{
	struct test_case *test = user_data;

	test->stage = TEST_STAGE_RUN;

	print_progress(test->name, COLOR_BLACK, "run");
	test->test_func(test->test_data);

	return FALSE;
}

static gboolean done_callback(gpointer user_data)
{
	struct test_case *test = user_data;

	test->end_time = g_timer_elapsed(test_timer, NULL);

	print_progress(test->name, COLOR_BLACK, "done");
	next_test_case();

	return FALSE;
}

void tester_pre_setup_complete(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_PRE_SETUP)
		return;

	g_idle_add(setup_callback, test);
}

void tester_pre_setup_failed(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_PRE_SETUP)
		return;

	print_progress(test->name, COLOR_RED, "pre setup failed");

	g_idle_add(done_callback, test);
}

void tester_setup_complete(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_SETUP)
		return;

	print_progress(test->name, COLOR_BLUE, "setup complete");

	g_idle_add(run_callback, test);
}

void tester_setup_failed(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_SETUP)
		return;

	test->stage = TEST_STAGE_POST_TEARDOWN;

	if (test->timeout_id > 0) {
		g_source_remove(test->timeout_id);
		test->timeout_id = 0;
	}

	print_progress(test->name, COLOR_RED, "setup failed");
	print_progress(test->name, COLOR_MAGENTA, "teardown");

	test->post_teardown_func(test->test_data);
}

static void test_result(enum test_result result)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_RUN)
		return;

	if (test->timeout_id > 0) {
		g_source_remove(test->timeout_id);
		test->timeout_id = 0;
	}

	test->result = result;
	switch (result) {
	case TEST_RESULT_PASSED:
		print_progress(test->name, COLOR_GREEN, "test passed");
		break;
	case TEST_RESULT_FAILED:
		print_progress(test->name, COLOR_RED, "test failed");
		break;
	case TEST_RESULT_NOT_RUN:
		print_progress(test->name, COLOR_YELLOW, "test not run");
		break;
	case TEST_RESULT_TIMED_OUT:
		print_progress(test->name, COLOR_RED, "test timed out");
		break;
	}

	if (test->teardown_id > 0)
		return;

	test->teardown_id = g_idle_add(teardown_callback, test);
}

void tester_test_passed(void)
{
	test_result(TEST_RESULT_PASSED);
}

void tester_test_failed(void)
{
	test_result(TEST_RESULT_FAILED);
}

void tester_test_abort(void)
{
	test_result(TEST_RESULT_NOT_RUN);
}

void tester_teardown_complete(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_TEARDOWN)
		return;

	test->stage = TEST_STAGE_POST_TEARDOWN;

	test->post_teardown_func(test->test_data);
}

void tester_teardown_failed(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_TEARDOWN)
		return;

	test->stage = TEST_STAGE_POST_TEARDOWN;

	tester_post_teardown_failed();
}

void tester_post_teardown_complete(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_POST_TEARDOWN)
		return;

	print_progress(test->name, COLOR_MAGENTA, "teardown complete");

	g_idle_add(done_callback, test);
}

void tester_post_teardown_failed(void)
{
	struct test_case *test;

	if (!test_current)
		return;

	test = test_current->data;

	if (test->stage != TEST_STAGE_POST_TEARDOWN)
		return;

	print_progress(test->name, COLOR_RED, "teardown failed");

	g_idle_add(done_callback, test);
}

static gboolean start_tester(gpointer user_data)
{
	test_timer = g_timer_new();

	next_test_case();

	return FALSE;
}

struct wait_data {
	unsigned int seconds;
	struct test_case *test;
	tester_wait_func_t func;
	void *user_data;
};

static gboolean wait_callback(gpointer user_data)
{
	struct wait_data *wait = user_data;
	struct test_case *test = wait->test;

	wait->seconds--;

	if (wait->seconds > 0) {
		print_progress(test->name, COLOR_BLACK, "%u seconds left",
								wait->seconds);
		return TRUE;
	}

	print_progress(test->name, COLOR_BLACK, "waiting done");

	wait->func(wait->user_data);

	free(wait);

	return FALSE;
}

void tester_wait(unsigned int seconds, tester_wait_func_t func,
							void *user_data)
{
	struct test_case *test;
	struct wait_data *wait;

	if (!func || seconds < 1)
		return;

	if (!test_current)
		return;

	test = test_current->data;

	wait = new0(struct wait_data, 1);
	wait->seconds = seconds;
	wait->test = test;
	wait->func = func;
	wait->user_data = user_data;

	g_timeout_add(1000, wait_callback, wait);

	print_progress(test->name, COLOR_BLACK, "waiting %u seconds", seconds);
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	static bool terminated = false;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (!terminated)
			g_main_loop_quit(main_loop);

		terminated = true;
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

bool tester_use_quiet(void)
{
	return option_quiet == TRUE ? true : false;
}

bool tester_use_debug(void)
{
	return option_debug == TRUE ? true : false;
}

static GOptionEntry options[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ "quiet", 'q', 0, G_OPTION_ARG_NONE, &option_quiet,
				"Run tests without logging" },
	{ "debug", 'd', 0, G_OPTION_ARG_NONE, &option_debug,
				"Run tests with debug output" },
	{ "list", 'l', 0, G_OPTION_ARG_NONE, &option_list,
				"Only list the tests to be run" },
	{ "prefix", 'p', 0, G_OPTION_ARG_STRING, &option_prefix,
				"Run tests matching provided prefix" },
	{ NULL },
};

void tester_init(int *argc, char ***argv)
{
	GOptionContext *context;
	GError *error = NULL;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, argc, argv, &error) == FALSE) {
		if (error != NULL) {
			g_printerr("%s\n", error->message);
			g_error_free(error);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		g_print("%s\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	main_loop = g_main_loop_new(NULL, FALSE);

	test_list = NULL;
	test_current = NULL;
}

int tester_run(void)
{
	guint signal;
	int ret;

	if (!main_loop)
		return EXIT_FAILURE;

	if (option_list) {
		g_main_loop_unref(main_loop);
		return EXIT_SUCCESS;
	}

	signal = setup_signalfd();

	g_idle_add(start_tester, NULL);
	g_main_loop_run(main_loop);

	g_source_remove(signal);

	g_main_loop_unref(main_loop);

	ret = tester_summarize();

	g_list_free_full(test_list, test_destroy);

	return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

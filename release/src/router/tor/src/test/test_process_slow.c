/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_process_slow.c
 * \brief Slow test cases for the Process API.
 */

#define MAINLOOP_PRIVATE
#include "orconfig.h"
#include "core/or/or.h"
#include "core/mainloop/mainloop.h"
#include "lib/evloop/compat_libevent.h"
#include "lib/process/process.h"
#include "lib/process/waitpid.h"
#include "test/test.h"

#ifndef BUILDDIR
#define BUILDDIR "."
#endif

#ifdef _WIN32
#define TEST_PROCESS "test-process.exe"
#else
#define TEST_PROCESS BUILDDIR "/src/test/test-process"
#endif /* defined(_WIN32) */

/** Timer that ticks once a second and stop the event loop after 5 ticks. */
static periodic_timer_t *main_loop_timeout_timer;

/** How many times have our timer ticked? */
static int timer_tick_count;

struct process_data_t {
  smartlist_t *stdout_data;
  smartlist_t *stderr_data;
  smartlist_t *stdin_data;
  process_exit_code_t exit_code;
  bool did_exit;
};

typedef struct process_data_t process_data_t;

static process_data_t *
process_data_new(void)
{
  process_data_t *process_data = tor_malloc_zero(sizeof(process_data_t));
  process_data->stdout_data = smartlist_new();
  process_data->stderr_data = smartlist_new();
  process_data->stdin_data = smartlist_new();
  return process_data;
}

static void
process_data_free(process_data_t *process_data)
{
  if (process_data == NULL)
    return;

  SMARTLIST_FOREACH(process_data->stdout_data, char *, x, tor_free(x));
  SMARTLIST_FOREACH(process_data->stderr_data, char *, x, tor_free(x));
  SMARTLIST_FOREACH(process_data->stdin_data, char *, x, tor_free(x));

  smartlist_free(process_data->stdout_data);
  smartlist_free(process_data->stderr_data);
  smartlist_free(process_data->stdin_data);
  tor_free(process_data);
}

static void
process_stdout_callback(process_t *process, const char *data, size_t size)
{
  tt_ptr_op(process, OP_NE, NULL);
  tt_ptr_op(data, OP_NE, NULL);
  tt_int_op(strlen(data), OP_EQ, size);

  process_data_t *process_data = process_get_data(process);
  smartlist_add(process_data->stdout_data, tor_strdup(data));

 done:
  return;
}

static void
process_stderr_callback(process_t *process, const char *data, size_t size)
{
  tt_ptr_op(process, OP_NE, NULL);
  tt_ptr_op(data, OP_NE, NULL);
  tt_int_op(strlen(data), OP_EQ, size);

  process_data_t *process_data = process_get_data(process);
  smartlist_add(process_data->stderr_data, tor_strdup(data));

 done:
  return;
}

static bool
process_exit_callback(process_t *process, process_exit_code_t exit_code)
{
  process_status_t status;

  tt_ptr_op(process, OP_NE, NULL);

  process_data_t *process_data = process_get_data(process);
  process_data->exit_code = exit_code;
  process_data->did_exit = true;

  /* Check if our process is still running? */
  status = process_get_status(process);
  tt_int_op(status, OP_EQ, PROCESS_STATUS_NOT_RUNNING);

 done:
  /* Do not free up our process_t. */
  return false;
}

#ifdef _WIN32
static const char *
get_win32_test_binary_path(void)
{
  static char buffer[MAX_PATH];

  /* Get the absolute path of our binary: \path\to\test-slow.exe. */
  GetModuleFileNameA(GetModuleHandle(0), buffer, sizeof(buffer));

  /* Find our process name. */
  char *offset = strstr(buffer, "test-slow.exe");
  tt_ptr_op(offset, OP_NE, NULL);

  /* Change test-slow.exe to test-process.exe. */
  memcpy(offset, TEST_PROCESS, strlen(TEST_PROCESS));

  return buffer;
 done:
  return NULL;
}
#endif /* defined(_WIN32) */

static void
main_loop_timeout_cb(periodic_timer_t *timer, void *data)
{
  /* Sanity check. */
  tt_ptr_op(timer, OP_EQ, main_loop_timeout_timer);
  tt_ptr_op(data, OP_NE, NULL);

  /* Our process data. */
  process_data_t *process_data = data;

  /* Our process did exit. */
  if (process_data->did_exit)
    tor_shutdown_event_loop_and_exit(0);

  /* Have we been called 10 times we exit the main loop. */
  timer_tick_count++;

  tt_int_op(timer_tick_count, OP_LT, 10);

#ifndef _WIN32
  /* Call waitpid callbacks. */
  notify_pending_waitpid_callbacks();
#endif

  return;
 done:
  /* Exit with an error. */
  tor_shutdown_event_loop_and_exit(-1);
}

static void
run_main_loop(process_data_t *process_data)
{
  int ret;

  /* Wake up after 1 seconds. */
  static const struct timeval interval = {1, 0};

  timer_tick_count = 0;
  main_loop_timeout_timer = periodic_timer_new(tor_libevent_get_base(),
                                               &interval,
                                               main_loop_timeout_cb,
                                               process_data);

  /* Run our main loop. */
  ret = run_main_loop_until_done();

  /* Clean up our main loop timeout timer. */
  tt_int_op(ret, OP_EQ, 0);

 done:
  periodic_timer_free(main_loop_timeout_timer);
}

static void
test_callbacks(void *arg)
{
  (void)arg;
  const char *filename = NULL;

#ifdef _WIN32
  filename = get_win32_test_binary_path();
#else
  filename = TEST_PROCESS;
#endif

  /* Process callback data. */
  process_data_t *process_data = process_data_new();

  /* Setup our process. */
  process_t *process = process_new(filename);
  process_set_data(process, process_data);
  process_set_stdout_read_callback(process, process_stdout_callback);
  process_set_stderr_read_callback(process, process_stderr_callback);
  process_set_exit_callback(process, process_exit_callback);

  /* Set environment variable. */
  process_set_environment(process, "TOR_TEST_ENV", "Hello, from Tor!");

  /* Add some arguments. */
  process_append_argument(process, "This is the first one");
  process_append_argument(process, "Second one");
  process_append_argument(process, "Third: Foo bar baz");

  /* Run our process. */
  process_status_t status;

  status = process_exec(process);
  tt_int_op(status, OP_EQ, PROCESS_STATUS_RUNNING);

  /* Write some lines to stdin. */
  process_printf(process, "Hi process!\r\n");
  process_printf(process, "Can you read more than one line?\n");
  process_printf(process, "Can you read partial ...");
  process_printf(process, " lines?\r\n");

  /* Start our main loop. */
  run_main_loop(process_data);

  /* We returned. Let's see what our event loop said. */
  tt_int_op(smartlist_len(process_data->stdout_data), OP_EQ, 12);
  tt_int_op(smartlist_len(process_data->stderr_data), OP_EQ, 3);
  tt_assert(process_data->did_exit);
  tt_u64_op(process_data->exit_code, OP_EQ, 0);

  /* Check stdout output. */
  char argv0_expected[256];
  tor_snprintf(argv0_expected, sizeof(argv0_expected),
               "argv[0] = '%s'", filename);

  tt_str_op(smartlist_get(process_data->stdout_data, 0), OP_EQ,
            argv0_expected);
  tt_str_op(smartlist_get(process_data->stdout_data, 1), OP_EQ,
            "argv[1] = 'This is the first one'");
  tt_str_op(smartlist_get(process_data->stdout_data, 2), OP_EQ,
            "argv[2] = 'Second one'");
  tt_str_op(smartlist_get(process_data->stdout_data, 3), OP_EQ,
            "argv[3] = 'Third: Foo bar baz'");
  tt_str_op(smartlist_get(process_data->stdout_data, 4), OP_EQ,
            "Environment variable TOR_TEST_ENV = 'Hello, from Tor!'");
  tt_str_op(smartlist_get(process_data->stdout_data, 5), OP_EQ,
            "Output on stdout");
  tt_str_op(smartlist_get(process_data->stdout_data, 6), OP_EQ,
            "This is a new line");
  tt_str_op(smartlist_get(process_data->stdout_data, 7), OP_EQ,
            "Partial line on stdout ...end of partial line on stdout");
  tt_str_op(smartlist_get(process_data->stdout_data, 8), OP_EQ,
            "Read line from stdin: 'Hi process!'");
  tt_str_op(smartlist_get(process_data->stdout_data, 9), OP_EQ,
            "Read line from stdin: 'Can you read more than one line?'");
  tt_str_op(smartlist_get(process_data->stdout_data, 10), OP_EQ,
            "Read line from stdin: 'Can you read partial ... lines?'");
  tt_str_op(smartlist_get(process_data->stdout_data, 11), OP_EQ,
            "We are done for here, thank you!");

  /* Check stderr output. */
  tt_str_op(smartlist_get(process_data->stderr_data, 0), OP_EQ,
            "Output on stderr");
  tt_str_op(smartlist_get(process_data->stderr_data, 1), OP_EQ,
            "This is a new line");
  tt_str_op(smartlist_get(process_data->stderr_data, 2), OP_EQ,
            "Partial line on stderr ...end of partial line on stderr");

 done:
  process_data_free(process_data);
  process_free(process);
}

static void
test_callbacks_terminate(void *arg)
{
  (void)arg;
  const char *filename = NULL;

#ifdef _WIN32
  filename = get_win32_test_binary_path();
#else
  filename = TEST_PROCESS;
#endif

  /* Process callback data. */
  process_data_t *process_data = process_data_new();

  /* Setup our process. */
  process_t *process = process_new(filename);
  process_set_data(process, process_data);
  process_set_exit_callback(process, process_exit_callback);

  /* Run our process. */
  process_status_t status;

  status = process_exec(process);
  tt_int_op(status, OP_EQ, PROCESS_STATUS_RUNNING);

  /* Zap our process. */
  bool success;

  success = process_terminate(process);
  tt_assert(success);

  /* Start our main loop. */
  run_main_loop(process_data);

  /* Check if we did exit. */
  tt_assert(process_data->did_exit);

 done:
  process_data_free(process_data);
  process_free(process);
}

static void
test_nonexistent_executable(void *arg)
{
  (void)arg;

  /* Process callback data. */
  process_data_t *process_data = process_data_new();

  /* Setup our process. */
  process_t *process = process_new("binary-does-not-exist");
  process_set_data(process, process_data);
  process_set_exit_callback(process, process_exit_callback);

  /* Run our process. */
  process_exec(process);

  /* Start our main loop. */
  run_main_loop(process_data);

  /* Ensure that the exit callback was actually called even though the binary
   * did not exist.
   */
  tt_assert(process_data->did_exit);

 done:
  process_data_free(process_data);
  process_free(process);
}

struct testcase_t slow_process_tests[] = {
  { "callbacks", test_callbacks, 0, NULL, NULL },
  { "callbacks_terminate", test_callbacks_terminate, 0, NULL, NULL },
  { "nonexistent_executable", test_nonexistent_executable, 0, NULL, NULL },
  END_OF_TESTCASES
};

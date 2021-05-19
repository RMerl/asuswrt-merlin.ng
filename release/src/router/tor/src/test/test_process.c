/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_process.c
 * \brief Test cases for the Process API.
 */

#include "orconfig.h"
#include "core/or/or.h"
#include "test/test.h"
#include "lib/process/env.h"

#define PROCESS_PRIVATE
#include "lib/process/process.h"
#define PROCESS_UNIX_PRIVATE
#include "lib/process/process_unix.h"
#define PROCESS_WIN32_PRIVATE
#include "lib/process/process_win32.h"

static const char *stdout_read_buffer;
static const char *stderr_read_buffer;

struct process_data_t {
  smartlist_t *stdout_data;
  smartlist_t *stderr_data;
  smartlist_t *stdin_data;
  process_exit_code_t exit_code;
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

static int
process_mocked_read_stdout(process_t *process, buf_t *buffer)
{
  (void)process;

  if (stdout_read_buffer != NULL) {
    buf_add_string(buffer, stdout_read_buffer);
    stdout_read_buffer = NULL;
  }

  return (int)buf_datalen(buffer);
}

static int
process_mocked_read_stderr(process_t *process, buf_t *buffer)
{
  (void)process;

  if (stderr_read_buffer != NULL) {
    buf_add_string(buffer, stderr_read_buffer);
    stderr_read_buffer = NULL;
  }

  return (int)buf_datalen(buffer);
}

static void
process_mocked_write_stdin(process_t *process, buf_t *buffer)
{
  const size_t size = buf_datalen(buffer);

  if (size == 0)
    return;

  char *data = tor_malloc_zero(size + 1);
  process_data_t *process_data = process_get_data(process);

  buf_get_bytes(buffer, data, size);
  smartlist_add(process_data->stdin_data, data);
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
  tt_ptr_op(process, OP_NE, NULL);

  process_data_t *process_data = process_get_data(process);
  process_data->exit_code = exit_code;

 done:
  /* Do not free up our process_t. */
  return false;
}

static void
test_default_values(void *arg)
{
  (void)arg;
  process_t *process = process_new("/path/to/nothing");

  /* We are not running by default. */
  tt_int_op(PROCESS_STATUS_NOT_RUNNING, OP_EQ, process_get_status(process));

  /* We use the line protocol by default. */
  tt_int_op(PROCESS_PROTOCOL_LINE, OP_EQ, process_get_protocol(process));

  /* We don't set any custom data by default. */
  tt_ptr_op(NULL, OP_EQ, process_get_data(process));

  /* Our command was given to the process_t's constructor in process_new(). */
  tt_str_op("/path/to/nothing", OP_EQ, process_get_command(process));

  /* Make sure we are listed in the list of processes. */
  tt_assert(smartlist_contains(process_get_all_processes(),
                               process));

  /* Default PID is 0. */
  tt_u64_op(0, OP_EQ, process_get_pid(process));

  /* Our arguments should be empty. */
  tt_int_op(0, OP_EQ,
            smartlist_len(process_get_arguments(process)));

 done:
  process_free(process);
}

static void
test_environment(void *arg)
{
  (void)arg;

  process_t *process = process_new("");
  process_environment_t *env = NULL;

  process_set_environment(process, "E", "F");
  process_set_environment(process, "C", "D");
  process_set_environment(process, "A", "B");

  env = process_get_environment(process);
  tt_mem_op(env->windows_environment_block, OP_EQ,
            "A=B\0C=D\0E=F\0", 12);
  tt_str_op(env->unixoid_environment_block[0], OP_EQ,
            "A=B");
  tt_str_op(env->unixoid_environment_block[1], OP_EQ,
            "C=D");
  tt_str_op(env->unixoid_environment_block[2], OP_EQ,
            "E=F");
  tt_ptr_op(env->unixoid_environment_block[3], OP_EQ,
            NULL);
  process_environment_free(env);

  /* Reset our environment. */
  smartlist_t *new_env = smartlist_new();
  smartlist_add(new_env, (char *)"FOO=bar");
  smartlist_add(new_env, (char *)"HELLO=world");

  process_reset_environment(process, new_env);
  smartlist_free(new_env);

  env = process_get_environment(process);
  tt_mem_op(env->windows_environment_block, OP_EQ,
            "FOO=bar\0HELLO=world\0", 20);
  tt_str_op(env->unixoid_environment_block[0], OP_EQ,
            "FOO=bar");
  tt_str_op(env->unixoid_environment_block[1], OP_EQ,
            "HELLO=world");
  tt_ptr_op(env->unixoid_environment_block[2], OP_EQ,
            NULL);

 done:
  process_environment_free(env);
  process_free(process);
}

static void
test_stringified_types(void *arg)
{
  (void)arg;

  /* process_protocol_t values. */
  tt_str_op("Raw", OP_EQ, process_protocol_to_string(PROCESS_PROTOCOL_RAW));
  tt_str_op("Line", OP_EQ, process_protocol_to_string(PROCESS_PROTOCOL_LINE));

  /* process_status_t values. */
  tt_str_op("not running", OP_EQ,
            process_status_to_string(PROCESS_STATUS_NOT_RUNNING));
  tt_str_op("running", OP_EQ,
            process_status_to_string(PROCESS_STATUS_RUNNING));
  tt_str_op("error", OP_EQ,
            process_status_to_string(PROCESS_STATUS_ERROR));

 done:
  return;
}

static void
test_line_protocol_simple(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);

  process_set_stdout_read_callback(process, process_stdout_callback);
  process_set_stderr_read_callback(process, process_stderr_callback);

  MOCK(process_read_stdout, process_mocked_read_stdout);
  MOCK(process_read_stderr, process_mocked_read_stderr);

  /* Make sure we are running with the line protocol. */
  tt_int_op(PROCESS_PROTOCOL_LINE, OP_EQ, process_get_protocol(process));

  tt_int_op(0, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = "Hello stdout\n";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = "Hello stderr\r\n";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Data should be ready. */
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stderr_data));

  /* Check if the data is correct. */
  tt_str_op(smartlist_get(process_data->stdout_data, 0), OP_EQ,
            "Hello stdout");
  tt_str_op(smartlist_get(process_data->stderr_data, 0), OP_EQ,
            "Hello stderr");

 done:
  process_data_free(process_data);
  process_free(process);

  UNMOCK(process_read_stdout);
  UNMOCK(process_read_stderr);
}

static void
test_line_protocol_multi(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);
  process_set_stdout_read_callback(process, process_stdout_callback);
  process_set_stderr_read_callback(process, process_stderr_callback);

  MOCK(process_read_stdout, process_mocked_read_stdout);
  MOCK(process_read_stderr, process_mocked_read_stderr);

  /* Make sure we are running with the line protocol. */
  tt_int_op(PROCESS_PROTOCOL_LINE, OP_EQ, process_get_protocol(process));

  tt_int_op(0, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = "Hello stdout\r\nOnion Onion Onion\nA B C D\r\n\r\n";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = "Hello stderr\nFoo bar baz\nOnion Onion Onion\n";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Data should be ready. */
  tt_int_op(4, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(3, OP_EQ, smartlist_len(process_data->stderr_data));

  /* Check if the data is correct. */
  tt_str_op(smartlist_get(process_data->stdout_data, 0), OP_EQ,
            "Hello stdout");
  tt_str_op(smartlist_get(process_data->stdout_data, 1), OP_EQ,
            "Onion Onion Onion");
  tt_str_op(smartlist_get(process_data->stdout_data, 2), OP_EQ,
            "A B C D");
  tt_str_op(smartlist_get(process_data->stdout_data, 3), OP_EQ,
            "");

  tt_str_op(smartlist_get(process_data->stderr_data, 0), OP_EQ,
            "Hello stderr");
  tt_str_op(smartlist_get(process_data->stderr_data, 1), OP_EQ,
            "Foo bar baz");
  tt_str_op(smartlist_get(process_data->stderr_data, 2), OP_EQ,
            "Onion Onion Onion");

 done:
  process_data_free(process_data);
  process_free(process);

  UNMOCK(process_read_stdout);
  UNMOCK(process_read_stderr);
}

static void
test_line_protocol_partial(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);
  process_set_stdout_read_callback(process, process_stdout_callback);
  process_set_stderr_read_callback(process, process_stderr_callback);

  MOCK(process_read_stdout, process_mocked_read_stdout);
  MOCK(process_read_stderr, process_mocked_read_stderr);

  /* Make sure we are running with the line protocol. */
  tt_int_op(PROCESS_PROTOCOL_LINE, OP_EQ, process_get_protocol(process));

  tt_int_op(0, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = "Hello stdout this is a partial line ...";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = "Hello stderr this is a partial line ...";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Data should NOT be ready. */
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = " the end\nAnother partial string goes here ...";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = " the end\nAnother partial string goes here ...";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Some data should be ready. */
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = " the end\nFoo bar baz\n";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = " the end\nFoo bar baz\n";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Some data should be ready. */
  tt_int_op(3, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(3, OP_EQ, smartlist_len(process_data->stderr_data));

  /* Check if the data is correct. */
  tt_str_op(smartlist_get(process_data->stdout_data, 0), OP_EQ,
                          "Hello stdout this is a partial line ... the end");
  tt_str_op(smartlist_get(process_data->stdout_data, 1), OP_EQ,
                          "Another partial string goes here ... the end");
  tt_str_op(smartlist_get(process_data->stdout_data, 2), OP_EQ,
                          "Foo bar baz");

  tt_str_op(smartlist_get(process_data->stderr_data, 0), OP_EQ,
                          "Hello stderr this is a partial line ... the end");
  tt_str_op(smartlist_get(process_data->stderr_data, 1), OP_EQ,
                          "Another partial string goes here ... the end");
  tt_str_op(smartlist_get(process_data->stderr_data, 2), OP_EQ,
                          "Foo bar baz");

 done:
  process_data_free(process_data);
  process_free(process);

  UNMOCK(process_read_stdout);
  UNMOCK(process_read_stderr);
}

static void
test_raw_protocol_simple(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);
  process_set_protocol(process, PROCESS_PROTOCOL_RAW);

  process_set_stdout_read_callback(process, process_stdout_callback);
  process_set_stderr_read_callback(process, process_stderr_callback);

  MOCK(process_read_stdout, process_mocked_read_stdout);
  MOCK(process_read_stderr, process_mocked_read_stderr);

  /* Make sure we are running with the raw protocol. */
  tt_int_op(PROCESS_PROTOCOL_RAW, OP_EQ, process_get_protocol(process));

  tt_int_op(0, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(0, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = "Hello stdout\n";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = "Hello stderr\n";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Data should be ready. */
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stderr_data));

  stdout_read_buffer = "Hello, again, stdout\nThis contains multiple lines";
  process_notify_event_stdout(process);
  tt_ptr_op(NULL, OP_EQ, stdout_read_buffer);

  stderr_read_buffer = "Hello, again, stderr\nThis contains multiple lines";
  process_notify_event_stderr(process);
  tt_ptr_op(NULL, OP_EQ, stderr_read_buffer);

  /* Data should be ready. */
  tt_int_op(2, OP_EQ, smartlist_len(process_data->stdout_data));
  tt_int_op(2, OP_EQ, smartlist_len(process_data->stderr_data));

  /* Check if the data is correct. */
  tt_str_op(smartlist_get(process_data->stdout_data, 0), OP_EQ,
            "Hello stdout\n");
  tt_str_op(smartlist_get(process_data->stdout_data, 1), OP_EQ,
            "Hello, again, stdout\nThis contains multiple lines");

  tt_str_op(smartlist_get(process_data->stderr_data, 0), OP_EQ,
            "Hello stderr\n");
  tt_str_op(smartlist_get(process_data->stderr_data, 1), OP_EQ,
            "Hello, again, stderr\nThis contains multiple lines");

 done:
  process_data_free(process_data);
  process_free(process);

  UNMOCK(process_read_stdout);
  UNMOCK(process_read_stderr);
}

static void
test_write_simple(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);

  MOCK(process_write_stdin, process_mocked_write_stdin);

  process_write(process, (uint8_t *)"Hello world\n", 12);
  process_notify_event_stdin(process);
  tt_int_op(1, OP_EQ, smartlist_len(process_data->stdin_data));

  process_printf(process, "Hello %s !\n", "moon");
  process_notify_event_stdin(process);
  tt_int_op(2, OP_EQ, smartlist_len(process_data->stdin_data));

 done:
  process_data_free(process_data);
  process_free(process);

  UNMOCK(process_write_stdin);
}

static void
test_exit_simple(void *arg)
{
  (void)arg;

  process_data_t *process_data = process_data_new();

  process_t *process = process_new("");
  process_set_data(process, process_data);
  process_set_exit_callback(process, process_exit_callback);

  /* Our default is 0. */
  tt_u64_op(0, OP_EQ, process_data->exit_code);

  /* Fake that we are a running process. */
  process_set_status(process, PROCESS_STATUS_RUNNING);
  tt_int_op(process_get_status(process), OP_EQ, PROCESS_STATUS_RUNNING);

  /* Fake an exit. */
  process_notify_event_exit(process, 1337);

  /* Check if our state changed and if our callback fired. */
  tt_int_op(process_get_status(process), OP_EQ, PROCESS_STATUS_NOT_RUNNING);
  tt_u64_op(1337, OP_EQ, process_data->exit_code);

 done:
  process_set_data(process, process_data);
  process_data_free(process_data);
  process_free(process);
}

static void
test_argv_simple(void *arg)
{
  (void)arg;

  process_t *process = process_new("/bin/cat");
  char **argv = NULL;

  /* Setup some arguments. */
  process_append_argument(process, "foo");
  process_append_argument(process, "bar");
  process_append_argument(process, "baz");

  /* Check the number of elements. */
  tt_int_op(3, OP_EQ,
            smartlist_len(process_get_arguments(process)));

  /* Let's try to convert it into a Unix style char **argv. */
  argv = process_get_argv(process);

  /* Check our values. */
  tt_str_op(argv[0], OP_EQ, "/bin/cat");
  tt_str_op(argv[1], OP_EQ, "foo");
  tt_str_op(argv[2], OP_EQ, "bar");
  tt_str_op(argv[3], OP_EQ, "baz");
  tt_ptr_op(argv[4], OP_EQ, NULL);

 done:
  tor_free(argv);
  process_free(process);
}

static void
test_unix(void *arg)
{
  (void)arg;
#ifndef _WIN32
  process_t *process = process_new("");

  /* On Unix all processes should have a Unix process handle. */
  tt_ptr_op(NULL, OP_NE, process_get_unix_process(process));

 done:
  process_free(process);
#endif /* !defined(_WIN32) */
}

static void
test_win32(void *arg)
{
  (void)arg;
#ifdef _WIN32
  process_t *process = process_new("");
  char *joined_argv = NULL;

  /* On Win32 all processes should have a Win32 process handle. */
  tt_ptr_op(NULL, OP_NE, process_get_win32_process(process));

  /* Based on some test cases from "Parsing C++ Command-Line Arguments" in
   * MSDN but we don't exercise all quoting rules because tor_join_win_cmdline
   * will try to only generate simple cases for the child process to parse;
   * i.e. we never embed quoted strings in arguments. */

  const char *argvs[][4] = {
    {"a", "bb", "CCC", NULL}, // Normal
    {NULL, NULL, NULL, NULL}, // Empty argument list
    {"", NULL, NULL, NULL}, // Empty argument
    {"\"a", "b\"b", "CCC\"", NULL}, // Quotes
    {"a\tbc", "dd  dd", "E", NULL}, // Whitespace
    {"a\\\\\\b", "de fg", "H", NULL}, // Backslashes
    {"a\\\"b", "\\c", "D\\", NULL}, // Backslashes before quote
    {"a\\\\b c", "d", "E", NULL}, // Backslashes not before quote
    { NULL } // Terminator
  };

  const char *cmdlines[] = {
    "a bb CCC",
    "",
    "\"\"",
    "\\\"a b\\\"b CCC\\\"",
    "\"a\tbc\" \"dd  dd\" E",
    "a\\\\\\b \"de fg\" H",
    "a\\\\\\\"b \\c D\\",
    "\"a\\\\b c\" d E",
    NULL // Terminator
  };

  int i;

  for (i=0; cmdlines[i]!=NULL; i++) {
    log_info(LD_GENERAL, "Joining argvs[%d], expecting <%s>", i, cmdlines[i]);
    joined_argv = tor_join_win_cmdline(argvs[i]);
    tt_str_op(cmdlines[i],OP_EQ, joined_argv);
    tor_free(joined_argv);
  }

 done:
  tor_free(joined_argv);
  process_free(process);
#endif /* defined(_WIN32) */
}

struct testcase_t process_tests[] = {
  { "default_values", test_default_values, TT_FORK, NULL, NULL },
  { "environment", test_environment, TT_FORK, NULL, NULL },
  { "stringified_types", test_stringified_types, TT_FORK, NULL, NULL },
  { "line_protocol_simple", test_line_protocol_simple, TT_FORK, NULL, NULL },
  { "line_protocol_multi", test_line_protocol_multi, TT_FORK, NULL, NULL },
  { "line_protocol_partial", test_line_protocol_partial, TT_FORK, NULL, NULL },
  { "raw_protocol_simple", test_raw_protocol_simple, TT_FORK, NULL, NULL },
  { "write_simple", test_write_simple, TT_FORK, NULL, NULL },
  { "exit_simple", test_exit_simple, TT_FORK, NULL, NULL },
  { "argv_simple", test_argv_simple, TT_FORK, NULL, NULL },
  { "unix", test_unix, TT_FORK, NULL, NULL },
  { "win32", test_win32, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};

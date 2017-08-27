/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "test_suite.h"

#include <unistd.h>

#include <utils/process.h>

START_TEST(test_retval_true)
{
	process_t *process;
	char *argv[] = {
#ifdef WIN32
		"C:\\Windows\\system32\\cmd.exe",
		"/C",
		"exit 0",
#else
		"/bin/sh",
		"-c",
		"true",
#endif
		NULL
	};
	int retval;

	process = process_start(argv, NULL, NULL, NULL, NULL, TRUE);
	ck_assert(process != NULL);
	ck_assert(process->wait(process, &retval));
	ck_assert_int_eq(retval, 0);
}
END_TEST

START_TEST(test_retval_false)
{
	process_t *process;
	char *argv[] = {
#ifdef WIN32
		"C:\\Windows\\system32\\cmd.exe",
		"/C",
		"exit 1",
#else
		"/bin/sh",
		"-c",
		"false",
#endif
		NULL
	};
	int retval;

	process = process_start(argv, NULL, NULL, NULL, NULL, TRUE);
	ck_assert(process != NULL);
	ck_assert(process->wait(process, &retval));
	ck_assert(retval != 0);
}
END_TEST

START_TEST(test_not_found)
{
	process_t *process;
	char *argv[] = {
		"/bin/does-not-exist",
		NULL
	};

	process = process_start(argv, NULL, NULL, NULL, NULL, TRUE);
	/* both is acceptable behavior */
	ck_assert(process == NULL || !process->wait(process, NULL));
}
END_TEST

START_TEST(test_echo)
{
	process_t *process;
	char *argv[] = {
#ifdef WIN32
		"C:\\Windows\\system32\\more.com",
#else
		"/bin/sh",
		"-c",
		"cat",
#endif
		NULL
	};
	int retval, in, out;
	char *msg = "test";
	char buf[strlen(msg) + 1];

	memset(buf, 0, strlen(msg) + 1);

	process = process_start(argv, NULL, &in, &out, NULL, TRUE);
	ck_assert(process != NULL);
	ck_assert_int_eq(write(in, msg, strlen(msg)), strlen(msg));
	ck_assert(close(in) == 0);
	ck_assert_int_eq(read(out, buf, strlen(msg) + 1), strlen(msg));
	ck_assert_str_eq(buf, msg);
	ck_assert(close(out) == 0);
	ck_assert(process->wait(process, &retval));
	ck_assert_int_eq(retval, 0);
}
END_TEST

START_TEST(test_echo_err)
{
	process_t *process;
	char *argv[] = {
#ifdef WIN32
		"C:\\Windows\\system32\\cmd.exe",
		"/C",
		"1>&2 C:\\Windows\\system32\\more.com",
#else
		"/bin/sh",
		"-c",
		"1>&2 cat",
#endif
		NULL
	};
	int retval, in, err;
	char *msg = "a longer test message";
	char buf[strlen(msg) + 1];

	memset(buf, 0, strlen(msg) + 1);

	process = process_start(argv, NULL, &in, NULL, &err, TRUE);
	ck_assert(process != NULL);
	ck_assert_int_eq(write(in, msg, strlen(msg)), strlen(msg));
	ck_assert(close(in) == 0);
	ck_assert_int_eq(read(err, buf, strlen(msg) + 1), strlen(msg));
	ck_assert_str_eq(buf, msg);
	ck_assert(close(err) == 0);
	ck_assert(process->wait(process, &retval));
	ck_assert_int_eq(retval, 0);
}
END_TEST

START_TEST(test_env)
{
	process_t *process;
	char *argv[] = {
#ifdef WIN32
		"C:\\Windows\\system32\\cmd.exe",
		"/C",
		"echo %A% %B%",
#else
		"/bin/sh",
		"-c",
		"/bin/echo -n $A $B",
#endif
		NULL
	};
	char *envp[] = {
		"A=atest",
		"B=bstring",
		NULL
	};
	int retval, out;
	char buf[64] = {};

	process = process_start(argv, envp, NULL, &out, NULL, TRUE);
	ck_assert(process != NULL);
	ck_assert(read(out, buf, sizeof(buf)) > 0);
#ifdef WIN32
	ck_assert_str_eq(buf, "atest bstring\r\n");
#else
	ck_assert_str_eq(buf, "atest bstring");
#endif
	ck_assert(close(out) == 0);
	ck_assert(process->wait(process, &retval));
	ck_assert_int_eq(retval, 0);
}
END_TEST

START_TEST(test_shell)
{
	process_t *process;
	int retval;

	process = process_start_shell(NULL, NULL, NULL, NULL, "exit %d", 3);
	ck_assert(process != NULL);
	ck_assert(process->wait(process, &retval));
	ck_assert_int_eq(retval, 3);
}
END_TEST

Suite *process_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("process");

	tc = tcase_create("return values");
	tcase_add_test(tc, test_retval_true);
	tcase_add_test(tc, test_retval_false);
	suite_add_tcase(s, tc);

	tc = tcase_create("not found");
	tcase_add_test(tc, test_not_found);
	suite_add_tcase(s, tc);

	tc = tcase_create("echo");
	tcase_add_test(tc, test_echo);
	tcase_add_test(tc, test_echo_err);
	suite_add_tcase(s, tc);

	tc = tcase_create("env");
	tcase_add_test(tc, test_env);
	suite_add_tcase(s, tc);

	tc = tcase_create("shell");
	tcase_add_test(tc, test_shell);
	suite_add_tcase(s, tc);

	return s;
}

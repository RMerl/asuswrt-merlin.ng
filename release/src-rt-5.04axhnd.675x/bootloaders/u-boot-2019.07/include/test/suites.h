/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __TEST_SUITES_H__
#define __TEST_SUITES_H__

struct unit_test;

/**
 * cmd_ut_category() - Run a category of unit tests
 *
 * @name:	Category name
 * @tests:	List of tests to run
 * @n_ents:	Number of tests in @tests
 * @argc:	Argument count provided. Must be >= 1. If this is 1 then all
 *		tests are run, otherwise only the one named @argv[1] is run.
 * @argv:	Arguments: argv[1] is the test to run (if @argc >= 2)
 * @return 0 if OK, CMD_RET_FAILURE on failure
 */
int cmd_ut_category(const char *name, struct unit_test *tests, int n_ents,
		    int argc, char * const argv[]);

int do_ut_bloblist(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_compression(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_env(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_lib(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_overlay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_time(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_unicode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __TEST_SUITES_H__ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __TEST_SUITES_H__
#define __TEST_SUITES_H__

struct cmd_tbl;
struct unit_test;

/**
 * cmd_ut_category() - Run a category of unit tests
 *
 * @name:	Category name
 * @prefix:	Prefix of test name
 * @tests:	List of tests to run
 * @n_ents:	Number of tests in @tests
 * @argc:	Argument count provided. Must be >= 1. If this is 1 then all
 *		tests are run, otherwise only the one named @argv[1] is run.
 * @argv:	Arguments: argv[1] is the test to run (if @argc >= 2)
 * Return: 0 if OK, CMD_RET_FAILURE on failure
 */
int cmd_ut_category(const char *name, const char *prefix,
		    struct unit_test *tests, int n_ents,
		    int argc, char *const argv[]);

int do_ut_addrmap(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
int do_ut_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_bootm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_bootstd(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
int do_ut_bloblist(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[]);
int do_ut_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_common(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_compression(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[]);
int do_ut_dm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_env(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_exit(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_fdt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_font(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_hush(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_lib(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_loadm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_log(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_mbr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_measurement(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_mem(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_optee(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_overlay(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
int do_ut_pci_mps(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
int do_ut_print(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_seama(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_setexpr(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);
int do_ut_str(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_time(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_ut_unicode(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[]);

#endif /* __TEST_SUITES_H__ */

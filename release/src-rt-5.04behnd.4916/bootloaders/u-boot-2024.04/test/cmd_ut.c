// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[]);

static int do_ut_info(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[]);

int cmd_ut_category(const char *name, const char *prefix,
		    struct unit_test *tests, int n_ents,
		    int argc, char *const argv[])
{
	const char *test_insert = NULL;
	int runs_per_text = 1;
	bool force_run = false;
	int ret;

	while (argc > 1 && *argv[1] == '-') {
		const char *str = argv[1];

		switch (str[1]) {
		case 'r':
			runs_per_text = dectoul(str + 2, NULL);
			break;
		case 'f':
			force_run = true;
			break;
		case 'I':
			test_insert = str + 2;
			break;
		}
		argv++;
		argc--;
	}

	ret = ut_run_list(name, prefix, tests, n_ents,
			  cmd_arg1(argc, argv), runs_per_text, force_run,
			  test_insert);

	return ret ? CMD_RET_FAILURE : 0;
}

static struct cmd_tbl cmd_ut_sub[] = {
	U_BOOT_CMD_MKENT(all, CONFIG_SYS_MAXARGS, 1, do_ut_all, "", ""),
	U_BOOT_CMD_MKENT(info, 1, 1, do_ut_info, "", ""),
#ifdef CONFIG_CMD_BDI
	U_BOOT_CMD_MKENT(bdinfo, CONFIG_SYS_MAXARGS, 1, do_ut_bdinfo, "", ""),
#endif
#ifdef CONFIG_UT_BOOTSTD
	U_BOOT_CMD_MKENT(bootstd, CONFIG_SYS_MAXARGS, 1, do_ut_bootstd,
			 "", ""),
#endif
#ifdef CONFIG_CMDLINE
	U_BOOT_CMD_MKENT(cmd, CONFIG_SYS_MAXARGS, 1, do_ut_cmd, "", ""),
#endif
	U_BOOT_CMD_MKENT(common, CONFIG_SYS_MAXARGS, 1, do_ut_common, "", ""),
#if defined(CONFIG_UT_DM)
	U_BOOT_CMD_MKENT(dm, CONFIG_SYS_MAXARGS, 1, do_ut_dm, "", ""),
#endif
#if defined(CONFIG_UT_ENV)
	U_BOOT_CMD_MKENT(env, CONFIG_SYS_MAXARGS, 1, do_ut_env, "", ""),
#endif
	U_BOOT_CMD_MKENT(exit, CONFIG_SYS_MAXARGS, 1, do_ut_exit, "", ""),
#ifdef CONFIG_CMD_FDT
	U_BOOT_CMD_MKENT(fdt, CONFIG_SYS_MAXARGS, 1, do_ut_fdt, "", ""),
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE
	U_BOOT_CMD_MKENT(font, CONFIG_SYS_MAXARGS, 1, do_ut_font, "", ""),
#endif
#ifdef CONFIG_UT_OPTEE
	U_BOOT_CMD_MKENT(optee, CONFIG_SYS_MAXARGS, 1, do_ut_optee, "", ""),
#endif
#ifdef CONFIG_UT_OVERLAY
	U_BOOT_CMD_MKENT(overlay, CONFIG_SYS_MAXARGS, 1, do_ut_overlay, "", ""),
#endif
#ifdef CONFIG_UT_LIB
	U_BOOT_CMD_MKENT(lib, CONFIG_SYS_MAXARGS, 1, do_ut_lib, "", ""),
#endif
#ifdef CONFIG_UT_LOG
	U_BOOT_CMD_MKENT(log, CONFIG_SYS_MAXARGS, 1, do_ut_log, "", ""),
#endif
#if defined(CONFIG_SANDBOX) && defined(CONFIG_CMD_MBR) && defined(CONFIG_CMD_MMC) \
        && defined(CONFIG_MMC_SANDBOX) && defined(CONFIG_MMC_WRITE)
	U_BOOT_CMD_MKENT(mbr, CONFIG_SYS_MAXARGS, 1, do_ut_mbr, "", ""),
#endif
	U_BOOT_CMD_MKENT(mem, CONFIG_SYS_MAXARGS, 1, do_ut_mem, "", ""),
#if defined(CONFIG_SANDBOX) && defined(CONFIG_CMD_SETEXPR)
	U_BOOT_CMD_MKENT(setexpr, CONFIG_SYS_MAXARGS, 1, do_ut_setexpr, "",
			 ""),
#endif
	U_BOOT_CMD_MKENT(print, CONFIG_SYS_MAXARGS, 1, do_ut_print, "", ""),
#ifdef CONFIG_UT_TIME
	U_BOOT_CMD_MKENT(time, CONFIG_SYS_MAXARGS, 1, do_ut_time, "", ""),
#endif
#if CONFIG_IS_ENABLED(UT_UNICODE) && !defined(API_BUILD)
	U_BOOT_CMD_MKENT(unicode, CONFIG_SYS_MAXARGS, 1, do_ut_unicode, "", ""),
#endif
#ifdef CONFIG_MEASURED_BOOT
	U_BOOT_CMD_MKENT(measurement, CONFIG_SYS_MAXARGS, 1, do_ut_measurement,
			 "", ""),
#endif
#ifdef CONFIG_SANDBOX
	U_BOOT_CMD_MKENT(compression, CONFIG_SYS_MAXARGS, 1, do_ut_compression,
			 "", ""),
	U_BOOT_CMD_MKENT(bloblist, CONFIG_SYS_MAXARGS, 1, do_ut_bloblist,
			 "", ""),
	U_BOOT_CMD_MKENT(bootm, CONFIG_SYS_MAXARGS, 1, do_ut_bootm, "", ""),
#endif
	U_BOOT_CMD_MKENT(str, CONFIG_SYS_MAXARGS, 1, do_ut_str, "", ""),
#ifdef CONFIG_CMD_ADDRMAP
	U_BOOT_CMD_MKENT(addrmap, CONFIG_SYS_MAXARGS, 1, do_ut_addrmap, "", ""),
#endif
#if CONFIG_IS_ENABLED(HUSH_PARSER)
	U_BOOT_CMD_MKENT(hush, CONFIG_SYS_MAXARGS, 1, do_ut_hush, "", ""),
#endif
#ifdef CONFIG_CMD_LOADM
	U_BOOT_CMD_MKENT(loadm, CONFIG_SYS_MAXARGS, 1, do_ut_loadm, "", ""),
#endif
#ifdef CONFIG_CMD_PCI_MPS
	U_BOOT_CMD_MKENT(pci_mps, CONFIG_SYS_MAXARGS, 1, do_ut_pci_mps, "", ""),
#endif
#ifdef CONFIG_CMD_SEAMA
	U_BOOT_CMD_MKENT(seama, CONFIG_SYS_MAXARGS, 1, do_ut_seama, "", ""),
#endif
};

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int i;
	int retval;
	int any_fail = 0;

	for (i = 1; i < ARRAY_SIZE(cmd_ut_sub); i++) {
		printf("----Running %s tests----\n", cmd_ut_sub[i].name);
		retval = cmd_ut_sub[i].cmd(cmdtp, flag, 1, &cmd_ut_sub[i].name);
		if (!any_fail)
			any_fail = retval;
	}

	return any_fail;
}

static int do_ut_info(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	printf("Test suites: %d\n", (int)ARRAY_SIZE(cmd_ut_sub));
	printf("Total tests: %d\n", (int)UNIT_TEST_ALL_COUNT());

	return 0;
}

static int do_ut(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "ut" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_ut_sub, ARRAY_SIZE(cmd_ut_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(ut,
	"[-r] [-f] [<suite>] - run unit tests\n"
	"   -r<runs>   Number of times to run each test\n"
	"   -f         Force 'manual' tests to run as well\n"
	"   <suite>    Test suite to run, or all\n"
	"\n"
	"\nOptions for <suite>:"
	"\nall - execute all enabled tests"
	"\ninfo - show info about tests"
#ifdef CONFIG_CMD_ADDRMAP
	"\naddrmap - very basic test of addrmap command"
#endif
#ifdef CONFIG_CMD_BDI
	"\nbdinfo - bdinfo command"
#endif
#ifdef CONFIG_SANDBOX
	"\nbloblist - bloblist implementation"
#endif
#ifdef CONFIG_BOOTSTD
	"\nbootstd - standard boot implementation"
#endif
#ifdef CONFIG_CMDLINE
	"\ncmd - test various commands"
#endif
#ifdef CONFIG_SANDBOX
	"\ncompression - compressors and bootm decompression"
#endif
#ifdef CONFIG_UT_DM
	"\ndm - driver model"
#endif
#ifdef CONFIG_UT_ENV
	"\nenv - environment"
#endif
#ifdef CONFIG_CMD_FDT
	"\nfdt - fdt command"
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE
	"\nfont - font command"
#endif
#if CONFIG_IS_ENABLED(HUSH_PARSER)
	"\nhush - Test hush behavior"
#endif
#ifdef CONFIG_CMD_LOADM
	"\nloadm - loadm command parameters and loading memory blob"
#endif
#ifdef CONFIG_UT_LIB
	"\nlib - library functions"
#endif
#ifdef CONFIG_UT_LOG
	"\nlog - logging functions"
#endif
	"\nmem - memory-related commands"
#ifdef CONFIG_UT_OPTEE
	"\noptee - test OP-TEE"
#endif
#ifdef CONFIG_UT_OVERLAY
	"\noverlay - device tree overlays"
#endif
#ifdef CONFIG_CMD_PCI_MPS
	"\npci_mps - PCI Express Maximum Payload Size"
#endif
	"\nprint  - printing things to the console"
	"\nsetexpr - setexpr command"
#ifdef CONFIG_SANDBOX
	"\nstr - basic test of string functions"
#endif
#ifdef CONFIG_CMD_SEAMA
	"\nseama - seama command parameters loading and decoding"
#endif
#ifdef CONFIG_UT_TIME
	"\ntime - very basic test of time functions"
#endif
#if defined(CONFIG_UT_UNICODE) && \
	!defined(CONFIG_SPL_BUILD) && !defined(API_BUILD)
	"\nunicode - Unicode functions"
#endif
	);

U_BOOT_CMD(
	ut, CONFIG_SYS_MAXARGS, 1, do_ut,
	"unit tests", ut_help_text
);

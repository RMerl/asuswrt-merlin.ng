// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <log.h>

static char log_fmt_chars[LOGF_COUNT] = "clFLfm";

static int do_log_level(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	if (argc > 1)
		gd->default_log_level = simple_strtol(argv[1], NULL, 10);
	else
		printf("Default log level: %d\n", gd->default_log_level);

	return 0;
}

static int do_log_format(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	int i;

	if (argc > 1) {
		const char *str = argv[1];

		if (!strcmp(str, "default")) {
			gd->log_fmt = LOGF_DEFAULT;
		} else if (!strcmp(str, "all")) {
			gd->log_fmt = LOGF_ALL;
		} else {
			gd->log_fmt = 0;
			for (; *str; str++) {
				char *ptr = strchr(log_fmt_chars, *str);

				if (!ptr) {
					printf("Invalid log char '%c'\n", *str);
					return CMD_RET_FAILURE;
				}
				gd->log_fmt |= 1 << (ptr - log_fmt_chars);
			}
		}
	} else {
		printf("Log format: ");
		for (i = 0; i < LOGF_COUNT; i++) {
			if (gd->log_fmt & (1 << i))
				printf("%c", log_fmt_chars[i]);
		}
		printf("\n");
	}

	return 0;
}

static int do_log_rec(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum log_category_t cat;
	enum log_level_t level;
	const char *file;
	uint line;
	const char *func;
	const char *msg;
	char *end;

	if (argc < 7)
		return CMD_RET_USAGE;
	cat = log_get_cat_by_name(argv[1]);
	level = simple_strtoul(argv[2], &end, 10);
	if (end == argv[2]) {
		level = log_get_level_by_name(argv[2]);

		if (level == LOGL_NONE) {
			printf("Invalid log level '%s'\n", argv[2]);
			return CMD_RET_USAGE;
		}
	}
	if (level >= LOGL_MAX) {
		printf("Invalid log level %u\n", level);
		return CMD_RET_USAGE;
	}
	file = argv[3];
	line = simple_strtoul(argv[4], NULL, 10);
	func = argv[5];
	msg = argv[6];
	if (_log(cat, level, file, line, func, "%s\n", msg))
		return CMD_RET_FAILURE;

	return 0;
}

static cmd_tbl_t log_sub[] = {
	U_BOOT_CMD_MKENT(level, CONFIG_SYS_MAXARGS, 1, do_log_level, "", ""),
#ifdef CONFIG_LOG_TEST
	U_BOOT_CMD_MKENT(test, 2, 1, do_log_test, "", ""),
#endif
	U_BOOT_CMD_MKENT(format, CONFIG_SYS_MAXARGS, 1, do_log_format, "", ""),
	U_BOOT_CMD_MKENT(rec, CONFIG_SYS_MAXARGS, 1, do_log_rec, "", ""),
};

static int do_log(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "log" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], log_sub, ARRAY_SIZE(log_sub));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char log_help_text[] =
	"level - get/set log level\n"
#ifdef CONFIG_LOG_TEST
	"log test - run log tests\n"
#endif
	"log format <fmt> - set log output format. <fmt> is a string where\n"
	"\teach letter indicates something that should be displayed:\n"
	"\tc=category, l=level, F=file, L=line number, f=function, m=msg\n"
	"\tor 'default', equivalent to 'fm', or 'all' for all\n"
	"log rec <category> <level> <file> <line> <func> <message> - "
		"output a log record"
	;
#endif

U_BOOT_CMD(
	log, CONFIG_SYS_MAXARGS, 1, do_log,
	"log system", log_help_text
);

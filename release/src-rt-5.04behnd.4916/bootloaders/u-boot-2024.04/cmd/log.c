// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <getopt.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>

static char log_fmt_chars[LOGF_COUNT] = "clFLfm";

static enum log_level_t parse_log_level(char *const arg)
{
	enum log_level_t ret;
	ulong level;

	if (!strict_strtoul(arg, 10, &level)) {
		if (level > _LOG_MAX_LEVEL) {
			printf("Only log levels <= %d are supported\n",
			       _LOG_MAX_LEVEL);
			return LOGL_NONE;
		}
		return level;
	}

	ret = log_get_level_by_name(arg);
	if (ret == LOGL_NONE)
		printf("Unknown log level \"%s\"\n", arg);
	return ret;
}

static int do_log_level(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	enum log_level_t log_level;

	if (argc > 1) {
		log_level = parse_log_level(argv[1]);

		if (log_level == LOGL_NONE)
			return CMD_RET_FAILURE;
		gd->default_log_level = log_level;
	} else {
		for (log_level = LOGL_FIRST; log_level <= _LOG_MAX_LEVEL;
		     log_level++)
			printf("%s%s\n", log_get_level_name(log_level),
			       log_level == gd->default_log_level ?
			       " (default)" : "");
	}

	return CMD_RET_SUCCESS;
}

static int do_log_categories(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	enum log_category_t cat;
	const char *name;

	for (cat = LOGC_FIRST; cat < LOGC_COUNT; cat++) {
		name = log_get_cat_name(cat);
		/*
		 * Invalid category names (e.g. <invalid> or <missing>) begin
		 * with '<'.
		 */
		if (name[0] == '<')
			continue;
		printf("%s\n", name);
	}

	return CMD_RET_SUCCESS;
}

static int do_log_drivers(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node)
		printf("%s\n", ldev->drv->name);

	return CMD_RET_SUCCESS;
}

static int do_log_filter_list(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	int opt;
	const char *drv_name = "console";
	struct getopt_state gs;
	struct log_filter *filt;
	struct log_device *ldev;

	getopt_init_state(&gs);
	while ((opt = getopt(&gs, argc, argv, "d:")) > 0) {
		switch (opt) {
		case 'd':
			drv_name = gs.arg;
			break;
		default:
			return CMD_RET_USAGE;
		}
	}

	if (gs.index != argc)
		return CMD_RET_USAGE;

	ldev = log_device_find_by_name(drv_name);
	if (!ldev) {
		printf("Could not find log device for \"%s\"\n", drv_name);
		return CMD_RET_FAILURE;
	}

	/*      <3> < 6  > <2+1 + 7 > <      16      > < unbounded... */
	printf("num policy level            categories files\n");
	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		printf("%3d %6.6s %s %-7.7s ", filt->filter_num,
		       filt->flags & LOGFF_DENY ? "deny" : "allow",
		       filt->flags & LOGFF_LEVEL_MIN ? ">=" : "<=",
		       log_get_level_name(filt->level));

		if (filt->flags & LOGFF_HAS_CAT) {
			int i;

			if (filt->cat_list[0] != LOGC_END)
				printf("%16.16s %s\n",
				       log_get_cat_name(filt->cat_list[0]),
				       filt->file_list ? filt->file_list : "");

			for (i = 1; i < LOGF_MAX_CATEGORIES &&
				    filt->cat_list[i] != LOGC_END; i++)
				printf("%21c %16.16s\n", ' ',
				       log_get_cat_name(filt->cat_list[i]));
		} else {
			printf("%16c %s\n", ' ',
			       filt->file_list ? filt->file_list : "");
		}
	}

	return CMD_RET_SUCCESS;
}

static int do_log_filter_add(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	bool level_set = false;
	bool print_num = false;
	bool type_set = false;
	char *file_list = NULL;
	const char *drv_name = "console";
	int opt, err;
	int cat_count = 0;
	int flags = 0;
	enum log_category_t cat_list[LOGF_MAX_CATEGORIES + 1];
	enum log_level_t level = LOGL_MAX;
	struct getopt_state gs;

	getopt_init_state(&gs);
	while ((opt = getopt(&gs, argc, argv, "Ac:d:Df:l:L:p")) > 0) {
		switch (opt) {
		case 'A':
#define do_type() do { \
			if (type_set) { \
				printf("Allow or deny set twice\n"); \
				return CMD_RET_USAGE; \
			} \
			type_set = true; \
} while (0)
			do_type();
			break;
		case 'c': {
			enum log_category_t cat;

			if (cat_count >= LOGF_MAX_CATEGORIES) {
				printf("Too many categories\n");
				return CMD_RET_FAILURE;
			}

			cat = log_get_cat_by_name(gs.arg);
			if (cat == LOGC_NONE) {
				printf("Unknown category \"%s\"\n", gs.arg);
				return CMD_RET_FAILURE;
			}

			cat_list[cat_count++] = cat;
			break;
		}
		case 'd':
			drv_name = gs.arg;
			break;
		case 'D':
			do_type();
			flags |= LOGFF_DENY;
			break;
		case 'f':
			file_list = gs.arg;
			break;
		case 'l':
#define do_level() do { \
			if (level_set) { \
				printf("Log level set twice\n"); \
				return CMD_RET_USAGE; \
			} \
			level = parse_log_level(gs.arg); \
			if (level == LOGL_NONE) \
				return CMD_RET_FAILURE; \
			level_set = true; \
} while (0)
			do_level();
			break;
		case 'L':
			do_level();
			flags |= LOGFF_LEVEL_MIN;
			break;
		case 'p':
			print_num = true;
			break;
		default:
			return CMD_RET_USAGE;
		}
	}

	if (gs.index != argc)
		return CMD_RET_USAGE;

	cat_list[cat_count] = LOGC_END;
	err = log_add_filter_flags(drv_name, cat_count ? cat_list : NULL, level,
				   file_list, flags);
	if (err < 0) {
		printf("Could not add filter (err = %d)\n", err);
		return CMD_RET_FAILURE;
	} else if (print_num) {
		printf("%d\n", err);
	}

	return CMD_RET_SUCCESS;
}

static int do_log_filter_remove(struct cmd_tbl *cmdtp, int flag, int argc,
				char *const argv[])
{
	bool all = false;
	int opt, err;
	ulong filter_num;
	const char *drv_name = "console";
	struct getopt_state gs;

	getopt_init_state(&gs);
	while ((opt = getopt(&gs, argc, argv, "ad:")) > 0) {
		switch (opt) {
		case 'a':
			all = true;
			break;
		case 'd':
			drv_name = gs.arg;
			break;
		default:
			return CMD_RET_USAGE;
		}
	}

	if (all) {
		struct log_filter *filt, *tmp_filt;
		struct log_device *ldev;

		if (gs.index != argc)
			return CMD_RET_USAGE;

		ldev = log_device_find_by_name(drv_name);
		if (!ldev) {
			printf("Could not find log device for \"%s\"\n",
			       drv_name);
			return CMD_RET_FAILURE;
		}

		list_for_each_entry_safe(filt, tmp_filt, &ldev->filter_head,
					 sibling_node) {
			list_del(&filt->sibling_node);
			free(filt);
		}
	} else {
		if (gs.index + 1 != argc)
			return CMD_RET_USAGE;

		if (strict_strtoul(argv[gs.index], 10, &filter_num)) {
			printf("Invalid filter number \"%s\"\n", argv[gs.index]);
			return CMD_RET_FAILURE;
		}

		err = log_remove_filter(drv_name, filter_num);
		if (err) {
			printf("Could not remove filter (err = %d)\n", err);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

static int do_log_format(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int i;

	if (argc > 1) {
		const char *str = argv[1];

		if (!strcmp(str, "default")) {
			gd->log_fmt = log_get_default_format();
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

static int do_log_rec(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
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
	level = dectoul(argv[2], &end);
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
	line = dectoul(argv[4], NULL);
	func = argv[5];
	msg = argv[6];
	if (_log(cat, level, file, line, func, "%s\n", msg))
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_LONGHELP(log,
	"level [<level>] - get/set log level\n"
	"categories - list log categories\n"
	"drivers - list log drivers\n"
	"log filter-list [OPTIONS] - list all filters for a log driver\n"
	"\t-d <driver> - Specify the log driver to list filters from; defaults\n"
	"\t              to console\n"
	"log filter-add [OPTIONS] - add a new filter to a driver\n"
	"\t-A - Allow messages matching this filter; mutually exclusive with -D\n"
	"\t     This is the default.\n"
	"\t-c <category> - Category to match; may be specified multiple times\n"
	"\t-d <driver> - Specify the log driver to add the filter to; defaults\n"
	"\t              to console\n"
	"\t-D - Deny messages matching this filter; mutually exclusive with -A\n"
	"\t-f <files_list> - A comma-separated list of files to match\n"
	"\t-l <level> - Match log levels less than or equal to <level>;\n"
	"\t             mutually-exclusive with -L\n"
	"\t-L <level> - Match log levels greather than or equal to <level>;\n"
	"\t             mutually-exclusive with -l\n"
	"\t-p - Print the filter number on success\n"
	"log filter-remove [OPTIONS] [<num>] - Remove filter number <num>\n"
	"\t-a - Remove ALL filters\n"
	"\t-d <driver> - Specify the log driver to remove the filter from;\n"
	"\t              defaults to console\n"
	"log format <fmt> - set log output format. <fmt> is a string where\n"
	"\teach letter indicates something that should be displayed:\n"
	"\tc=category, l=level, F=file, L=line number, f=function, m=msg\n"
	"\tor 'default', or 'all' for all\n"
	"log rec <category> <level> <file> <line> <func> <message> - "
		"output a log record");

U_BOOT_CMD_WITH_SUBCMDS(log, "log system", log_help_text,
	U_BOOT_SUBCMD_MKENT(level, 2, 1, do_log_level),
	U_BOOT_SUBCMD_MKENT(categories, 1, 1, do_log_categories),
	U_BOOT_SUBCMD_MKENT(drivers, 1, 1, do_log_drivers),
	U_BOOT_SUBCMD_MKENT(filter-list, 3, 1, do_log_filter_list),
	U_BOOT_SUBCMD_MKENT(filter-add, CONFIG_SYS_MAXARGS, 1,
			    do_log_filter_add),
	U_BOOT_SUBCMD_MKENT(filter-remove, 4, 1, do_log_filter_remove),
	U_BOOT_SUBCMD_MKENT(format, 2, 1, do_log_format),
	U_BOOT_SUBCMD_MKENT(rec, 7, 1, do_log_rec),
);

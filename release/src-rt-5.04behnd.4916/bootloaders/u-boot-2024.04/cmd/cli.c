// SPDX-License-Identifier: GPL-2.0+

#include <cli.h>
#include <command.h>
#include <string.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *gd_flags_to_parser_name(void)
{
	if (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		return "old";
	if (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		return "modern";
	return NULL;
}

static int do_cli_get(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	const char *current = gd_flags_to_parser_name();

	if (!current) {
		printf("current cli value is not valid, this should not happen!\n");
		return CMD_RET_FAILURE;
	}

	printf("%s\n", current);

	return CMD_RET_SUCCESS;
}

static int parser_string_to_gd_flags(const char *parser)
{
	if (!strcmp(parser, "old"))
		return GD_FLG_HUSH_OLD_PARSER;
	if (!strcmp(parser, "modern"))
		return GD_FLG_HUSH_MODERN_PARSER;
	return -1;
}

static int gd_flags_to_parser_config(int flag)
{
	if (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		return CONFIG_VAL(HUSH_OLD_PARSER);
	if (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		return CONFIG_VAL(HUSH_MODERN_PARSER);
	return -1;
}

static void reset_parser_gd_flags(void)
{
	gd->flags &= ~GD_FLG_HUSH_OLD_PARSER;
	gd->flags &= ~GD_FLG_HUSH_MODERN_PARSER;
}

static int do_cli_set(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	char *parser_name;
	int parser_config;
	int parser_flag;

	if (argc < 2)
		return CMD_RET_USAGE;

	parser_name = argv[1];

	parser_flag = parser_string_to_gd_flags(parser_name);
	if (parser_flag == -1) {
		printf("Bad value for parser name: %s\n", parser_name);
		return CMD_RET_USAGE;
	}

	parser_config = gd_flags_to_parser_config(parser_flag);
	switch (parser_config) {
	case -1:
		printf("Bad value for parser flags: %d\n", parser_flag);
		return CMD_RET_FAILURE;
	case 0:
		printf("Want to set current parser to %s, but its code was not compiled!\n",
			parser_name);
		return CMD_RET_FAILURE;
	}

	reset_parser_gd_flags();
	gd->flags |= parser_flag;

	cli_init();
	cli_loop();

	/* cli_loop() should never return. */
	return CMD_RET_FAILURE;
}

static struct cmd_tbl parser_sub[] = {
	U_BOOT_CMD_MKENT(get, 1, 1, do_cli_get, "", ""),
	U_BOOT_CMD_MKENT(set, 2, 1, do_cli_set, "", ""),
};

static int do_cli(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "parser" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], parser_sub, ARRAY_SIZE(parser_sub));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#if CONFIG_IS_ENABLED(SYS_LONGHELP)
static char cli_help_text[] =
	"get - print current cli\n"
	"set - set the current cli, possible value are: old, modern"
	;
#endif

U_BOOT_CMD(cli, 3, 1, do_cli,
	   "cli",
#if CONFIG_IS_ENABLED(SYS_LONGHELP)
	   cli_help_text
#endif
);

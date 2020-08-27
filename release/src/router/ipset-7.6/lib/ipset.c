/* Copyright 2007-2010 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <assert.h>				/* assert */
#include <ctype.h>				/* isspace */
#include <errno.h>				/* errno */
#include <stdarg.h>				/* va_* */
#include <stdbool.h>				/* bool */
#include <stdio.h>				/* printf */
#include <stdlib.h>				/* exit */
#include <string.h>				/* str* */

#include <config.h>

#include <libipset/debug.h>			/* D() */
#include <libipset/linux_ip_set.h>		/* IPSET_CMD_* */
#include <libipset/icmp.h>			/* id_to_icmp */
#include <libipset/icmpv6.h>			/* id_to_icmpv6 */
#include <libipset/data.h>			/* enum ipset_data */
#include <libipset/types.h>			/* IPSET_*_ARG */
#include <libipset/session.h>			/* ipset_envopt_parse */
#include <libipset/parse.h>			/* ipset_parse_family */
#include <libipset/print.h>			/* ipset_print_family */
#include <libipset/utils.h>			/* STREQ */
#include <libipset/ipset.h>			/* prototypes */

static char program_name[] = PACKAGE;
static char program_version[] = PACKAGE_VERSION;

#define MAX_CMDLINE_CHARS			1024
#define MAX_ARGS				32

/* The ipset structure */
struct ipset {
	ipset_custom_errorfn custom_error;
		/* Custom error message function */
	ipset_standard_errorfn standard_error;
		/* Standard error message function */
	struct ipset_session *session;		/* Session */
	uint32_t restore_line;			/* Restore lineno */
	bool interactive;			/* "Interactive" CLI */
	bool full_io;				/* Use session ios */
	bool no_vhi;				/* No version/help/interactive */
	char cmdline[MAX_CMDLINE_CHARS];	/* For restore mode */
	char *newargv[MAX_ARGS];
	int newargc;
	const char *filename;			/* Input/output filename */
};

/* Commands and environment options */

const struct ipset_commands ipset_commands[] = {
	/* Order is important */

	{	/* c[reate], --create, n[ew], -N */
		.cmd = IPSET_CMD_CREATE,
		.name = { "create", "new", "-N" },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "SETNAME TYPENAME [type-specific-options]\n"
			"        Create a new set",
	},
	{	/* a[dd], --add, -A  */
		.cmd = IPSET_CMD_ADD,
		.name = { "add", "-A", NULL },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "SETNAME ENTRY\n"
			"        Add entry to the named set",
	},
	{	/* d[el], --del, -D */
		.cmd = IPSET_CMD_DEL,
		.name = { "del", "-D", NULL },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "SETNAME ENTRY\n"
			"        Delete entry from the named set",
	},
	{	/* t[est], --test, -T */
		.cmd = IPSET_CMD_TEST,
		.name = { "test", "-T", NULL },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "SETNAME ENTRY\n"
			"        Test entry in the named set",
	},
	{	/* des[troy], --destroy, x, -X */
		.cmd = IPSET_CMD_DESTROY,
		.name = { "destroy", "x", "-X" },
		.has_arg = IPSET_OPTIONAL_ARG,
		.help = "[SETNAME]\n"
			"        Destroy a named set or all sets",
	},
	{	/* l[ist], --list, -L */
		.cmd = IPSET_CMD_LIST,
		.name = { "list", "-L", NULL },
		.has_arg = IPSET_OPTIONAL_ARG,
		.help = "[SETNAME]\n"
			"        List the entries of a named set or all sets",
	},
	{	/* s[save], --save, -S */
		.cmd = IPSET_CMD_SAVE,
		.name = { "save", "-S", NULL },
		.has_arg = IPSET_OPTIONAL_ARG,
		.help = "[SETNAME]\n"
			"        Save the named set or all sets to stdout",
	},
	{	/* r[estore], --restore, -R */
		.cmd = IPSET_CMD_RESTORE,
		.name = { "restore", "-R", NULL },
		.has_arg = IPSET_NO_ARG,
		.help = "\n"
			"        Restore a saved state",
	},
	{	/* f[lush], --flush, -F */
		.cmd = IPSET_CMD_FLUSH,
		.name = { "flush", "-F", NULL },
		.has_arg = IPSET_OPTIONAL_ARG,
		.help = "[SETNAME]\n"
			"        Flush a named set or all sets",
	},
	{	/* ren[ame], --rename, e, -E */
		.cmd = IPSET_CMD_RENAME,
		.name = { "rename", "e", "-E" },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "FROM-SETNAME TO-SETNAME\n"
			"        Rename two sets",
	},
	{	/* sw[ap], --swap, w, -W */
		.cmd = IPSET_CMD_SWAP,
		.name = { "swap", "w", "-W" },
		.has_arg = IPSET_MANDATORY_ARG2,
		.help = "FROM-SETNAME TO-SETNAME\n"
			"        Swap the contect of two existing sets",
	},
	{	/* h[elp, --help, -H */
		.cmd = IPSET_CMD_HELP,
		.name = { "help", "-h", "-H" },
		.has_arg = IPSET_OPTIONAL_ARG,
		.help = "[TYPENAME]\n"
			"        Print help, and settype specific help",
	},
	{	/* v[ersion], --version, -V */
		.cmd = IPSET_CMD_VERSION,
		.name = { "version", "-v", "-V" },
		.has_arg = IPSET_NO_ARG,
		.help = "\n"
			"        Print version information",
	},
	{	/* q[uit] */
		.cmd = IPSET_CMD_QUIT,
		.name = { "quit", NULL },
		.has_arg = IPSET_NO_ARG,
		.help = "\n"
			"        Quit interactive mode",
	},
	{ },
};

/**
 * ipset_match_cmd - try to match as a prefix or letter-command
 * @arg: possible command string
 * @name: command and it's aliases
 *
 * Returns true if @arg is a known command.
 */
bool
ipset_match_cmd(const char *arg, const char * const name[])
{
	size_t len, skip = 0;
	int i;

	assert(arg);
	assert(name && name[0]);

	/* Ignore two leading dashes */
	if (arg[0] == '-' && arg[1] == '-')
		skip = 2;

	len = strlen(arg);
	if (len <= skip || (len == 1 && arg[0] == '-'))
		return false;

	for (i = 0; i < IPSET_CMD_ALIASES && name[i] != NULL; i++) {
		/* New command name options */
		if (STRNEQ(arg + skip, name[i], len - skip))
			return true;
	}
	return false;
}

/* Used up so far
 *
 *	-A		add
 *	-D		del
 *	-E		rename
 *	-f		-file
 *	-F		flush
 *	-h		help
 *	-H		help
 *	-L		list
 *	-n		-name
 *	-N		create
 *	-o		-output
 *	-r		-resolve
 *	-R		restore
 *	-s		-sorted
 *	-S		save
 *	-t		-terse
 *	-T		test
 *	-q		-quiet
 *	-X		destroy
 *	-v		version
 *	-V		version
 *	-W		swap
 *	-!		-exist
 */

const struct ipset_envopts ipset_envopts[] = {
	{ .name = { "-o", "-output" },
	  .has_arg = IPSET_MANDATORY_ARG,	.flag = IPSET_OPT_MAX,
	  .parse = ipset_parse_output,
	  .help = "plain|save|xml\n"
		  "       Specify output mode for listing sets.\n"
		  "       Default value for \"list\" command is mode \"plain\"\n"
		  "       and for \"save\" command is mode \"save\".",
	},
	{ .name = { "-s", "-sorted" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_SORTED,
	  .help = "\n"
		  "        Print elements sorted (if supported by the set type).",
	},
	{ .name = { "-q", "-quiet" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_QUIET,
	  .help = "\n"
		  "        Suppress any notice or warning message.",
	},
	{ .name = { "-r", "-resolve" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_RESOLVE,
	  .help = "\n"
		  "        Try to resolve IP addresses in the output (slow!)",
	},
	{ .name = { "-!", "-exist" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_EXIST,
	  .help = "\n"
		  "        Ignore errors when creating or adding sets or\n"
		  "        elements that do exist or when deleting elements\n"
		  "        that don't exist.",
	},
	{ .name = { "-n", "-name" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_LIST_SETNAME,
	  .help = "\n"
		  "        When listing, just list setnames from the kernel.\n",
	},
	{ .name = { "-t", "-terse" },
	  .parse = ipset_envopt_parse,
	  .has_arg = IPSET_NO_ARG,	.flag = IPSET_ENV_LIST_HEADER,
	  .help = "\n"
		  "        When listing, list setnames and set headers\n"
		  "        from kernel only.",
	},
	{ .name = { "-f", "-file" },
	  .parse = ipset_parse_filename,
	  .has_arg = IPSET_MANDATORY_ARG,	.flag = IPSET_OPT_MAX,
	  .help = "\n"
		  "        Read from the given file instead of standard\n"
		  "        input (restore) or write to given file instead\n"
		  "        of standard output (list/save).",
	},
	{ },
};

/**
 * ipset_match_option - strict option matching
 * @arg: possible option string
 * @name: known option and it's alias
 *
 * Two leading dashes are ignored.
 *
 * Returns true if @arg is a known option.
 */
bool
ipset_match_option(const char *arg, const char * const name[])
{
	assert(arg);
	assert(name && name[0]);

	/* Skip two leading dashes */
	if (arg[0] == '-' && arg[1] == '-')
		arg++, arg++;

	return STREQ(arg, name[0]) ||
	       (name[1] != NULL && STREQ(arg, name[1]));
}

/**
 * ipset_match_envopt - strict envopt matching
 * @arg: possible envopt string
 * @name: known envopt and it's alias
 *
 * One leading dash is ignored.
 *
 * Returns true if @arg is a known envopt.
 */
bool
ipset_match_envopt(const char *arg, const char * const name[])
{
	assert(arg);
	assert(name && name[0]);

	/* Skip one leading dash */
	if (arg[0] == '-' && arg[1] == '-')
		arg++;

	return STREQ(arg, name[0]) ||
	       (name[1] != NULL && STREQ(arg, name[1]));
}

static void
ipset_shift_argv(int *argc, char *argv[], int from)
{
	int i;

	assert(*argc >= from + 1);

	for (i = from + 1; i <= *argc; i++)
		argv[i-1] = argv[i];
	(*argc)--;
	return;
}

/**
 * ipset_port_usage - prints the usage for the port parameter
 *
 * Print the usage for the port parameter to stdout.
 */
void
ipset_port_usage(void)
{
	int i;
	const char *name;

	printf("      [PROTO:]PORT is a valid pattern of the following:\n"
	       "           PORTNAME         TCP port name from /etc/services\n"
	       "           PORTNUMBER       TCP port number identifier\n"
	       "           tcp|sctp|udp|udplite:PORTNAME|PORTNUMBER\n"
	       "           icmp:CODENAME    supported ICMP codename\n"
	       "           icmp:TYPE/CODE   ICMP type/code value\n"
	       "           icmpv6:CODENAME  supported ICMPv6 codename\n"
	       "           icmpv6:TYPE/CODE ICMPv6 type/code value\n"
	       "           PROTO:0          all other protocols\n\n");

	printf("           Supported ICMP codenames:\n");
	i = 0;
	while ((name = id_to_icmp(i++)) != NULL)
		printf("               %s\n", name);
	printf("           Supported ICMPv6 codenames:\n");
	i = 0;
	while ((name = id_to_icmpv6(i++)) != NULL)
		printf("               %s\n", name);
}

/**
 * ipset_parse_filename - parse filename
 * @ipset: ipset structure
 * @opt: option kind of the data
 * @str: string to parse
 *
 * Parse filename of "-file" option, which can be used once only.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_parse_filename(struct ipset *ipset,
		     int opt UNUSED, const char *str)
{
	void *p = ipset_session_printf_private(ipset->session);

	if (ipset->filename)
		return ipset->custom_error(ipset, p, IPSET_PARAMETER_PROBLEM,
			"-file option cannot be used when full io is activated");
	ipset->filename = str;

	return 0;
}

/**
 * ipset_parse_output - parse output format name
 * @ipset: ipset structure
 * @opt: option kind of the data
 * @str: string to parse
 *
 * Parse output format names and set session mode.
 * The value is stored in the session.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_parse_output(struct ipset *ipset,
		   int opt UNUSED, const char *str)
{
	struct ipset_session *session;

	assert(ipset);
	assert(str);

	session = ipset_session(ipset);
	if (STREQ(str, "plain"))
		return ipset_session_output(session, IPSET_LIST_PLAIN);
	else if (STREQ(str, "xml"))
		return ipset_session_output(session, IPSET_LIST_XML);
	else if (STREQ(str, "save"))
		return ipset_session_output(session, IPSET_LIST_SAVE);

	return ipset_err(session,
		"Syntax error: unknown output mode '%s'", str);
}

/**
 * ipset_envopt_parse - parse/set environment option
 * @ipset: ipset structure
 * @opt: environment option
 * @arg: option argument (unused)
 *
 * Parse and set an environment option.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_envopt_parse(struct ipset *ipset, int opt,
		   const char *arg UNUSED)
{
	struct ipset_session *session;
	assert(ipset);

	session = ipset_session(ipset);
	switch (opt) {
	case IPSET_ENV_SORTED:
	case IPSET_ENV_QUIET:
	case IPSET_ENV_RESOLVE:
	case IPSET_ENV_EXIST:
	case IPSET_ENV_LIST_SETNAME:
	case IPSET_ENV_LIST_HEADER:
		ipset_envopt_set(session, opt);
		return 0;
	default:
		break;
	}
	return -1;
}

static int __attribute__((format(printf, 4, 5)))
default_custom_error(struct ipset *ipset, void *p UNUSED,
		     int status, const char *msg, ...)
{
	struct ipset_session *session = ipset_session(ipset);
	bool is_interactive = ipset_is_interactive(ipset);
	bool quiet = !is_interactive &&
		     session &&
		     ipset_envopt_test(session, IPSET_ENV_QUIET);

	if (status && msg && !quiet) {
		va_list args;

		fprintf(stderr, "%s v%s: ", program_name, program_version);
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
		if (status != IPSET_SESSION_PROBLEM)
			fprintf(stderr, "\n");

		if (status == IPSET_PARAMETER_PROBLEM)
			fprintf(stderr,
				"Try `%s help' for more information.\n",
				program_name);
	}
	/* Ignore errors in interactive mode */
	if (status && is_interactive) {
		if (session)
			ipset_session_report_reset(session);
		return -1;
	}

	D("status: %u", status);
	ipset_fini(ipset);
	exit(status > IPSET_VERSION_PROBLEM ? IPSET_OTHER_PROBLEM : status);
	/* Unreached */
	return -1;
}

static int
default_standard_error(struct ipset *ipset, void *p)
{
	struct ipset_session *session = ipset_session(ipset);
	bool is_interactive = ipset_is_interactive(ipset);
	enum ipset_err_type err_type = ipset_session_report_type(session);

	if ((err_type == IPSET_WARNING || err_type == IPSET_NOTICE) &&
	    !ipset_envopt_test(session, IPSET_ENV_QUIET))
		fprintf(stderr, "%s%s",
			err_type == IPSET_WARNING ? "Warning: " : "",
			ipset_session_report_msg(session));
	if (err_type == IPSET_ERROR)
		return ipset->custom_error(ipset, p,
				IPSET_SESSION_PROBLEM, "%s",
				ipset_session_report_msg(session));

	if (!is_interactive) {
		ipset_fini(ipset);
		/* Warnings are not errors */
		exit(err_type <= IPSET_WARNING ? 0 : IPSET_OTHER_PROBLEM);
	}

	ipset_session_report_reset(session);
	return -1;
}

static void
default_help(void)
{
	const struct ipset_commands *c;
	const struct ipset_envopts *opt = ipset_envopts;

	printf("%s v%s\n\n"
	       "Usage: %s [options] COMMAND\n\nCommands:\n",
	       program_name, program_version, program_name);

	for (c = ipset_commands; c->cmd; c++)
		printf("%s %s\n", c->name[0], c->help);
	printf("\nOptions:\n");

	while (opt->flag) {
		if (opt->help)
			printf("%s %s\n", opt->name[0], opt->help);
		opt++;
	}
}

static void
reset_argv(struct ipset *ipset)
{
	int i;

	/* Reset */
	for (i = 1; i < ipset->newargc; i++) {
		if (ipset->newargv[i])
			free(ipset->newargv[i]);
		ipset->newargv[i] = NULL;
	}
	ipset->newargc = 1;
}

/* Build fake argv from parsed line */
static int
build_argv(struct ipset *ipset, char *buffer)
{
	void *p = ipset_session_printf_private(ipset->session);
	char *tmp, *arg;
	int i;
	bool quoted = false;

	reset_argv(ipset);
	arg = calloc(strlen(buffer) + 1, sizeof(*buffer));
	if (!arg)
		return ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
					   "Cannot allocate memory.");
	for (tmp = buffer, i = 0; *tmp; tmp++) {
		if ((ipset->newargc + 1) ==
		    (int)(sizeof(ipset->newargv)/sizeof(char *))) {
			free(arg);
			return ipset->custom_error(ipset,
					p, IPSET_PARAMETER_PROBLEM,
					"Line is too long to parse.");
		}
		switch (*tmp) {
		case '"':
			quoted = !quoted;
			if (*(tmp+1))
				continue;
			break;
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			if (!quoted)
				break;
			arg[i++] = *tmp;
			continue;
		default:
			arg[i++] = *tmp;
			if (*(tmp+1))
				continue;
			break;
		}
		if (!*(tmp+1) && quoted) {
			free(arg);
			return ipset->custom_error(ipset,
				p, IPSET_PARAMETER_PROBLEM,
				"Missing close quote!");
		}
		if (!*arg)
			continue;
		ipset->newargv[ipset->newargc] =
			calloc(strlen(arg) + 1, sizeof(*arg));
		if (!ipset->newargv[ipset->newargc]) {
			free(arg);
			return ipset->custom_error(ipset,
				p, IPSET_OTHER_PROBLEM,
				"Cannot allocate memory.");
		}
		ipset_strlcpy(ipset->newargv[ipset->newargc++],
			      arg, strlen(arg) + 1);
		memset(arg, 0, strlen(arg) + 1);
		i = 0;
	}

	free(arg);
	return 0;
}

static int
restore(struct ipset *ipset)
{
	struct ipset_session *session = ipset_session(ipset);
	int ret = 0;
	FILE *f = stdin;	/* Default from stdin */

	if (ipset->filename) {
		ret = ipset_session_io_normal(session, ipset->filename,
					      IPSET_IO_INPUT);
		if (ret < 0)
			return ret;
		f = ipset_session_io_stream(session, IPSET_IO_INPUT);
	}
	return ipset_parse_stream(ipset, f);
}

static bool do_parse(const struct ipset_arg *arg, bool family)
{
	return !((family == true) ^ (arg->opt == IPSET_OPT_FAMILY));
}

static int
call_parser(struct ipset *ipset, int *argc, char *argv[],
	    const struct ipset_type *type, enum ipset_adt cmd, bool family)
{
	void *p = ipset_session_printf_private(ipset->session);
	const struct ipset_arg *arg;
	const char *optstr;
	const struct ipset_type *t = type;
	uint8_t revision = type->revision;
	int ret = 0, i = 1, j;

	/* Currently CREATE and ADT may have got additional arguments */
	if (type->cmd[cmd].args[0] == IPSET_ARG_NONE && *argc > 1)
		return ipset->custom_error(ipset,
				p, IPSET_PARAMETER_PROBLEM,
				"Unknown argument: `%s'", argv[i]);

	while (*argc > i) {
		ret = -1;
		for (j = 0; type->cmd[cmd].args[j] != IPSET_ARG_NONE; j++) {
			arg = ipset_keyword(type->cmd[cmd].args[j]);
			D("argc: %u, %s vs %s", i, argv[i], arg->name[0]);
			if (!(ipset_match_option(argv[i], arg->name)))
				continue;

			optstr = argv[i];
			/* Matched option */
			D("match %s, argc %u, i %u, %s",
			  arg->name[0], *argc, i + 1,
			  do_parse(arg, family) ? "parse" : "skip");
			i++;
			ret = 0;
			switch (arg->has_arg) {
			case IPSET_MANDATORY_ARG:
				if (*argc - i < 1)
					return ipset->custom_error(ipset, p,
						IPSET_PARAMETER_PROBLEM,
						"Missing mandatory argument "
						"of option `%s'",
						arg->name[0]);
				/* Fall through */
			case IPSET_OPTIONAL_ARG:
				if (*argc - i >= 1) {
					if (do_parse(arg, family)) {
						ret = ipset_call_parser(
							ipset->session,
							arg, argv[i]);
						if (ret < 0)
							return ret;
					}
					i++;
					break;
				}
				/* Fall through */
			default:
				if (do_parse(arg, family)) {
					ret = ipset_call_parser(
						ipset->session, arg, optstr);
					if (ret < 0)
						return ret;
				}
			}
			break;
		}
		if (ret < 0)
			goto err_unknown;
	}
	if (!family)
		*argc = 0;
	return ret;

err_unknown:
	while ((type = ipset_type_higher_rev(t)) != t) {
		for (j = 0; type->cmd[cmd].args[j] != IPSET_ARG_NONE; j++) {
			arg = ipset_keyword(type->cmd[cmd].args[j]);
			D("argc: %u, %s vs %s", i, argv[i], arg->name[0]);
			if (ipset_match_option(argv[i], arg->name))
				return ipset->custom_error(ipset, p,
					IPSET_PARAMETER_PROBLEM,
					"Argument `%s' is supported in the kernel module "
					"of the set type %s starting from the revision %u "
					"and you have installed revision %u only. "
					"Your kernel is behind your ipset utility.",
					argv[i], type->name,
					type->revision, revision);
		}
		t = type;
	}
	return ipset->custom_error(ipset, p, IPSET_PARAMETER_PROBLEM,
				   "Unknown argument: `%s'", argv[i]);
}

static enum ipset_adt
cmd2cmd(int cmd)
{
	switch (cmd) {
	case IPSET_CMD_ADD:
		return IPSET_ADD;
	case IPSET_CMD_DEL:
		return IPSET_DEL;
	case IPSET_CMD_TEST:
		return IPSET_TEST;
	case IPSET_CMD_CREATE:
		return IPSET_CREATE;
	default:
		return 0;
	}
}

static void
check_mandatory(struct ipset *ipset,
		const struct ipset_type *type, enum ipset_cmd command)
{
	enum ipset_adt cmd = cmd2cmd(command);
	struct ipset_session *session = ipset->session;
	void *p = ipset_session_printf_private(session);
	uint64_t flags = ipset_data_flags(ipset_session_data(session));
	uint64_t mandatory = type->cmd[cmd].need;
	const struct ipset_arg *arg;
	int i;

	/* Range can be expressed by ip/cidr */
	if (flags & IPSET_FLAG(IPSET_OPT_CIDR))
		flags |= IPSET_FLAG(IPSET_OPT_IP_TO);

	mandatory &= ~flags;
	if (!mandatory)
		return;
	if (type->cmd[cmd].args[0] == IPSET_ARG_NONE) {
		ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
			"There are missing mandatory flags "
			"but can't check them. "
			"It's a bug, please report the problem.");
		return;
	}

	for (i = 0; type->cmd[cmd].args[i] != IPSET_ARG_NONE; i++) {
		arg = ipset_keyword(type->cmd[cmd].args[i]);
		if (mandatory & IPSET_FLAG(arg->opt)) {
			ipset->custom_error(ipset, p, IPSET_PARAMETER_PROBLEM,
				   "Mandatory option `%s' is missing",
				   arg->name[0]);
			return;
		}
	}
}

static const char *
cmd2name(enum ipset_cmd cmd)
{
	const struct ipset_commands *c;

	for (c = ipset_commands; c->cmd; c++)
		if (cmd == c->cmd)
			return c->name[0];
	return "unknown command";
}

static const char *
session_family(struct ipset_session *session)
{
	switch (ipset_data_family(ipset_session_data(session))) {
	case NFPROTO_IPV4:
		return "inet";
	case NFPROTO_IPV6:
		return "inet6";
	default:
		return "unspec";
	}
}

static void
check_allowed(struct ipset *ipset,
	      const struct ipset_type *type, enum ipset_cmd command)
{
	struct ipset_session *session = ipset->session;
	void *p = ipset_session_printf_private(session);
	uint64_t flags = ipset_data_flags(ipset_session_data(session));
	enum ipset_adt cmd = cmd2cmd(command);
	uint64_t allowed = type->cmd[cmd].full;
	uint64_t cmdflags = command == IPSET_CMD_CREATE
				? IPSET_CREATE_FLAGS : IPSET_ADT_FLAGS;
	const struct ipset_arg *arg;
	enum ipset_opt i;
	int j;

	/* Range can be expressed by ip/cidr or from-to */
	if (allowed & IPSET_FLAG(IPSET_OPT_IP_TO))
		allowed |= IPSET_FLAG(IPSET_OPT_CIDR);

	for (i = IPSET_OPT_IP; i < IPSET_OPT_FLAGS; i++) {
		if (!(cmdflags & IPSET_FLAG(i)) ||
		    (allowed & IPSET_FLAG(i)) ||
		    !(flags & IPSET_FLAG(i)))
			continue;
		/* Not allowed element-expressions */
		switch (i) {
		case IPSET_OPT_CIDR:
			ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
				"IP/CIDR range is not allowed in command %s "
				"with set type %s and family %s",
				cmd2name(command), type->name,
				session_family(ipset->session));
			return;
		case IPSET_OPT_IP_TO:
			ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
				"FROM-TO IP range is not allowed in command %s "
				"with set type %s and family %s",
				cmd2name(command), type->name,
				session_family(ipset->session));
			return;
		case IPSET_OPT_PORT_TO:
			ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
				"FROM-TO port range is not allowed in command %s "
				"with set type %s and family %s",
				cmd2name(command), type->name,
				session_family(ipset->session));
			return;
		default:
			break;
		}
		/* Other options */
		if (type->cmd[cmd].args[0] == IPSET_ARG_NONE) {
			ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
				"There are not allowed options (%u) "
				"but option list is empty. "
				"It's a bug, please report the problem.", i);
			return;
		}
		for (j = 0; type->cmd[cmd].args[j] != IPSET_ARG_NONE; j++) {
			arg = ipset_keyword(type->cmd[cmd].args[j]);
			if (arg->opt != i)
				continue;
			ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
				"%s parameter is not allowed in command %s "
				"with set type %s and family %s",
				arg->name[0],
				cmd2name(command), type->name,
				session_family(ipset->session));
			return;
		}
		ipset->custom_error(ipset, p, IPSET_OTHER_PROBLEM,
			"There are not allowed options (%u) "
			"but can't resolve them. "
			"It's a bug, please report the problem.", i);
		return;
	}
}

static const struct ipset_type *
type_find(const char *name)
{
	const struct ipset_type *t = ipset_types();

	while (t) {
		if (ipset_match_typename(name, t))
			return t;
		t = t->next;
	}
	return NULL;
}

static enum ipset_adt cmd_help_order[] = {
	IPSET_CREATE,
	IPSET_ADD,
	IPSET_DEL,
	IPSET_TEST,
	IPSET_CADT_MAX,
};

static const char *cmd_prefix[] = {
	[IPSET_CREATE] = "create SETNAME",
	[IPSET_ADD]    = "add    SETNAME",
	[IPSET_DEL]    = "del    SETNAME",
	[IPSET_TEST]   = "test   SETNAME",
};

/* Workhorses */

/**
 * ipset_parse_argv - parse and argv array and execute the command
 * @ipset: ipset structure
 * @argc: length of the array
 * @argv: array of strings
 *
 * Parse an array of strings and execute the ipset command.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_parse_argv(struct ipset *ipset, int oargc, char *oargv[])
{
	int ret = 0;
	enum ipset_cmd cmd = IPSET_CMD_NONE;
	int i;
	char *arg0 = NULL, *arg1 = NULL;
	const struct ipset_envopts *opt;
	const struct ipset_commands *command;
	const struct ipset_type *type;
	struct ipset_session *session = ipset->session;
	void *p = ipset_session_printf_private(session);
	int argc = oargc;
	char *argv[MAX_ARGS] = {};

	/* We need a local copy because of ipset_shift_argv */
	memcpy(argv, oargv, sizeof(char *) * argc);

	/* Set session lineno to report parser errors correctly */
	ipset_session_lineno(session, ipset->restore_line);

	/* Commandline parsing, somewhat similar to that of 'ip' */

	/* First: parse core options */
	for (opt = ipset_envopts; opt->flag; opt++) {
		for (i = 1; i < argc; ) {
			if (!ipset_match_envopt(argv[i], opt->name)) {
				i++;
				continue;
			}
			/* Shift off matched option */
			ipset_shift_argv(&argc, argv, i);
			switch (opt->has_arg) {
			case IPSET_MANDATORY_ARG:
				if (i + 1 > argc)
					return ipset->custom_error(ipset, p,
						IPSET_PARAMETER_PROBLEM,
						"Missing mandatory argument "
						"to option %s",
						opt->name[0]);
				/* Fall through */
			case IPSET_OPTIONAL_ARG:
				if (i + 1 <= argc) {
					ret = opt->parse(ipset, opt->flag,
							 argv[i]);
					if (ret < 0)
						return ipset->standard_error(ipset, p);
					ipset_shift_argv(&argc, argv, i);
				}
				break;
			case IPSET_NO_ARG:
				ret = opt->parse(ipset, opt->flag,
						 opt->name[0]);
				if (ret < 0)
					return ipset->standard_error(ipset, p);
				break;
			default:
				break;
			}
		}
	}

	/* Second: parse command */
	for (command = ipset_commands;
		 argc > 1 && command->cmd && cmd == IPSET_CMD_NONE;
	     command++) {
		if (!ipset_match_cmd(argv[1], command->name))
			continue;

		if (ipset->restore_line != 0 &&
		    (command->cmd == IPSET_CMD_RESTORE ||
		     command->cmd == IPSET_CMD_VERSION ||
		     command->cmd == IPSET_CMD_HELP))
			return ipset->custom_error(ipset, p,
				IPSET_PARAMETER_PROBLEM,
				"Command `%s' is invalid "
				"in restore mode.",
				command->name[0]);
		if (ipset->interactive && command->cmd == IPSET_CMD_RESTORE) {
			printf("Restore command is not supported "
			       "in interactive mode\n");
			return 0;
		}

		/* Shift off matched command arg */
		ipset_shift_argv(&argc, argv, 1);
		cmd = command->cmd;
		switch (command->has_arg) {
		case IPSET_MANDATORY_ARG:
		case IPSET_MANDATORY_ARG2:
			if (argc < 2)
				return ipset->custom_error(ipset, p,
					IPSET_PARAMETER_PROBLEM,
					"Missing mandatory argument "
					"to command %s",
					command->name[0]);
			/* Fall through */
		case IPSET_OPTIONAL_ARG:
			arg0 = argv[1];
			if (argc >= 2)
				/* Shift off first arg */
				ipset_shift_argv(&argc, argv, 1);
			break;
		default:
			break;
		}
		if (command->has_arg == IPSET_MANDATORY_ARG2) {
			if (argc < 2)
				return ipset->custom_error(ipset, p,
					IPSET_PARAMETER_PROBLEM,
					"Missing second mandatory "
					"argument to command %s",
					command->name[0]);
			arg1 = argv[1];
			/* Shift off second arg */
			ipset_shift_argv(&argc, argv, 1);
		}
		break;
	}

	/* Third: catch interactive mode, handle help, version */
	switch (cmd) {
	case IPSET_CMD_NONE:
		if (ipset->interactive) {
			printf("No command specified\n");
			if (session)
				ipset_envopt_parse(ipset, 0, "reset");
			return 0;
		}
		if (argc > 1 && STREQ(argv[1], "-")) {
			if (ipset->no_vhi)
				return 0;
			ipset->interactive = true;
			printf("%s> ", program_name);
			while (fgets(ipset->cmdline,
				     sizeof(ipset->cmdline), stdin)) {
				/* Execute line: ignore soft errors */
				if (ipset_parse_line(ipset, ipset->cmdline) < 0)
					ipset->standard_error(ipset, p);
				printf("%s> ", program_name);
			}
			return ipset->custom_error(ipset, p,
						   IPSET_NO_PROBLEM, NULL);
		}
		if (argc > 1)
			return ipset->custom_error(ipset,
				p, IPSET_PARAMETER_PROBLEM,
				"No command specified: unknown argument %s",
				argv[1]);
		return ipset->custom_error(ipset, p, IPSET_PARAMETER_PROBLEM,
					   "No command specified.");
	case IPSET_CMD_VERSION:
		if (ipset->no_vhi)
			return 0;
		printf("%s v%s, protocol version: %u\n",
		       program_name, program_version, IPSET_PROTOCOL);
		/* Check kernel protocol version */
		ipset_cmd(session, IPSET_CMD_NONE, 0);
		if (ipset_session_report_type(session) != IPSET_NO_ERROR)
			ipset->standard_error(ipset, p);
		if (ipset->interactive)
			return 0;
		return ipset->custom_error(ipset, p, IPSET_NO_PROBLEM, NULL);
	case IPSET_CMD_HELP:
		if (ipset->no_vhi)
			return 0;
		default_help();

		if (ipset->interactive ||
		    !ipset_envopt_test(session, IPSET_ENV_QUIET)) {
			if (arg0) {
				const struct ipset_arg *arg;
				int k;

				/* Type-specific help, without kernel checking */
				type = type_find(arg0);
				if (!type)
					return ipset->custom_error(ipset, p,
						IPSET_PARAMETER_PROBLEM,
						"Unknown settype: `%s'", arg0);
				printf("\n%s type specific options:\n\n", type->name);
				for (i = 0; cmd_help_order[i] != IPSET_CADT_MAX; i++) {
					cmd = cmd_help_order[i];
					printf("%s %s %s\n",
						cmd_prefix[cmd], type->name, type->cmd[cmd].help);
					for (k = 0; type->cmd[cmd].args[k] != IPSET_ARG_NONE; k++) {
						arg = ipset_keyword(type->cmd[cmd].args[k]);
						if (!arg->help || arg->help[0] == '\0')
							continue;
						printf("               %s\n", arg->help);
					}
				}
				printf("\n%s\n", type->usage);
				if (type->usagefn)
					type->usagefn();
				if (type->family == NFPROTO_UNSPEC)
					printf("\nType %s is family neutral.\n",
					       type->name);
				else if (type->family == NFPROTO_IPSET_IPV46)
					printf("\nType %s supports inet "
					       "and inet6.\n",
					       type->name);
				else
					printf("\nType %s supports family "
					       "%s only.\n",
					       type->name,
					       type->family == NFPROTO_IPV4
						? "inet" : "inet6");
			} else {
				printf("\nSupported set types:\n");
				type = ipset_types();
				while (type) {
					printf("    %s\t%s%u\t%s\n",
					       type->name,
					       strlen(type->name) < 12 ? "\t" : "",
					       type->revision,
					       type->description);
					type = type->next;
				}
			}
		}
		if (ipset->interactive)
			return 0;
		return ipset->custom_error(ipset, p, IPSET_NO_PROBLEM, NULL);
	case IPSET_CMD_QUIT:
		return ipset->custom_error(ipset, p, IPSET_NO_PROBLEM, NULL);
	default:
		break;
	}

	/* Forth: parse command args and issue the command */
	switch (cmd) {
	case IPSET_CMD_CREATE:
		/* Args: setname typename [type specific options] */
		ret = ipset_parse_setname(session, IPSET_SETNAME, arg0);
		if (ret < 0)
			return ipset->standard_error(ipset, p);

		ret = ipset_parse_typename(session, IPSET_OPT_TYPENAME, arg1);
		if (ret < 0)
			return ipset->standard_error(ipset, p);

		type = ipset_type_get(session, cmd);
		if (type == NULL)
			return ipset->standard_error(ipset, p);

		/* Parse create options: first check INET family */
		ret = call_parser(ipset, &argc, argv, type, IPSET_CREATE, true);
		if (ret < 0)
			return ipset->standard_error(ipset, p);
		else if (ret)
			return ret;

		/* Parse create options: then check all options */
		ret = call_parser(ipset, &argc, argv, type, IPSET_CREATE, false);
		if (ret < 0)
			return ipset->standard_error(ipset, p);
		else if (ret)
			return ret;

		/* Check mandatory, then allowed options */
		check_mandatory(ipset, type, cmd);
		check_allowed(ipset, type, cmd);

		break;
	case IPSET_CMD_LIST:
	case IPSET_CMD_SAVE:
		if (ipset->filename != NULL) {
			ret = ipset_session_io_normal(session,
					ipset->filename, IPSET_IO_OUTPUT);
			if (ret < 0)
				return ret;
		}
		/* Fall through to parse optional setname */
	case IPSET_CMD_DESTROY:
	case IPSET_CMD_FLUSH:
		/* Args: [setname] */
		if (arg0) {
			ret = ipset_parse_setname(session,
						  IPSET_SETNAME, arg0);
			if (ret < 0)
				return ipset->standard_error(ipset, p);
		}
		break;

	case IPSET_CMD_RENAME:
	case IPSET_CMD_SWAP:
		/* Args: from-setname to-setname */
		ret = ipset_parse_setname(session, IPSET_SETNAME, arg0);
		if (ret < 0)
			return ipset->standard_error(ipset, p);
		ret = ipset_parse_setname(session, IPSET_OPT_SETNAME2, arg1);
		if (ret < 0)
			return ipset->standard_error(ipset, p);
		break;

	case IPSET_CMD_RESTORE:
		/* Restore mode */
		if (argc > 1)
			return ipset->custom_error(ipset,
				p, IPSET_PARAMETER_PROBLEM,
				"Unknown argument %s", argv[1]);
		return restore(ipset);
	case IPSET_CMD_ADD:
	case IPSET_CMD_DEL:
	case IPSET_CMD_TEST:
		D("ADT: setname %s", arg0);
		/* Args: setname ip [options] */
		ret = ipset_parse_setname(session, IPSET_SETNAME, arg0);
		if (ret < 0)
			return ipset->standard_error(ipset, p);

		type = ipset_type_get(session, cmd);
		if (type == NULL)
			return ipset->standard_error(ipset, p);

		ret = ipset_parse_elem(session, type->last_elem_optional, arg1);
		if (ret < 0)
			return ipset->standard_error(ipset, p);

		/* Parse additional ADT options */
		ret = call_parser(ipset, &argc, argv, type, cmd2cmd(cmd), false);
		if (ret < 0)
			return ipset->standard_error(ipset, p);
		else if (ret)
			return ret;

		/* Check mandatory, then allowed options */
		check_mandatory(ipset, type, cmd);
		check_allowed(ipset, type, cmd);

		break;
	default:
		break;
	}

	if (argc > 1)
		return ipset->custom_error(ipset, p, IPSET_PARAMETER_PROBLEM,
			"Unknown argument %s", argv[1]);
	ret = ipset_cmd(session, cmd, ipset->restore_line);
	D("ret %d", ret);
	/* In the case of warning, the return code is success */
	if (ret < 0 || ipset_session_report_type(session) > IPSET_NO_ERROR)
		ipset->standard_error(ipset, p);

	return ret;
}

/**
 * ipset_parse_line - parse a string as a command line and execute it
 * @ipset: ipset structure
 * @line: string of line
 *
 * Parse a string as a command line and execute the ipset command.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_parse_line(struct ipset *ipset, char *line)
{
	char *c = line;
	int ret;

	reset_argv(ipset);

	while (isspace(c[0]))
		c++;
	if (c[0] == '\0' || c[0] == '#') {
		if (ipset->interactive)
			printf("%s> ", program_name);
		return 0;
	}
	/* Build fake argv, argc */
	ret = build_argv(ipset, c);
	if (ret < 0)
		return ret;
	/* Parse and execute line */
	return ipset_parse_argv(ipset, ipset->newargc, ipset->newargv);
}

/**
 * ipset_parse_stream - parse an stream and execute the commands
 * @ipset: ipset structure
 * @f: stream
 *
 * Parse an already opened file as stream and execute the commands.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_parse_stream(struct ipset *ipset, FILE *f)
{
	struct ipset_session *session = ipset_session(ipset);
	void *p = ipset_session_printf_private(session);
	int ret = 0;
	char *c;

	while (fgets(ipset->cmdline, sizeof(ipset->cmdline), f)) {
		ipset->restore_line++;
		c = ipset->cmdline;
		while (isspace(c[0]))
			c++;
		if (c[0] == '\0' || c[0] == '#')
			continue;
		else if (STREQ(c, "COMMIT\n") || STREQ(c, "COMMIT\r\n")) {
			ret = ipset_commit(ipset->session);
			if (ret < 0)
				ipset->standard_error(ipset, p);
			continue;
		}
		/* Build faked argv, argc */
		ret = build_argv(ipset, c);
		if (ret < 0)
			return ret;

		/* Execute line */
		ret = ipset_parse_argv(ipset, ipset->newargc, ipset->newargv);
		if (ret < 0)
			ipset->standard_error(ipset, p);
	}
	/* implicit "COMMIT" at EOF */
	ret = ipset_commit(ipset->session);
	if (ret < 0)
		ipset->standard_error(ipset, p);

	return ret;
}

/**
 * ipset_session - returns the session pointer of an ipset structure
 * @ipset: ipset structure
 *
 * Returns the session pointer of an ipset structure.
 */
struct ipset_session *
ipset_session(struct ipset *ipset)
{
	return ipset->session;
}

/**
 * ipset_is_interactive - is the interactive mode enabled?
 * @ipset: ipset structure
 *
 * Returns true if the interactive mode is enabled.
 */
bool
ipset_is_interactive(struct ipset *ipset)
{
	return ipset->interactive;
}

/**
 * ipset_custom_printf - set custom print functions
 * @ipset: ipset structure
 * @custom_error: custom error function
 * @standard_error: standard error function
 * @print_outfn: output/printing function
 * @p: pointer to private data area
 *
 * The function makes possible to set custom error and
 * output functions for the library. The private data
 * pointer can be used to pass arbitrary data to these functions.
 * If a function argument is NULL, the default printing function is set.
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_custom_printf(struct ipset *ipset,
		    ipset_custom_errorfn custom_error,
		    ipset_standard_errorfn standard_error,
		    ipset_print_outfn print_outfn,
		    void *p)
{
	ipset->no_vhi = !!(custom_error || standard_error || print_outfn);
	ipset->custom_error =
		custom_error ? custom_error : default_custom_error;
	ipset->standard_error =
		standard_error ? standard_error : default_standard_error;

	return ipset_session_print_outfn(ipset->session, print_outfn, p);
}

/**
 * ipset_init - initialize ipset library interface
 *
 * Initialize the ipset library interface.
 *
 * Returns the created ipset structure for success or NULL for failure.
 */
struct ipset *
ipset_init(void)
{
	struct ipset *ipset;

	ipset = calloc(1, sizeof(struct ipset));
	if (ipset == NULL)
		return NULL;
	ipset->newargv[0] =
		calloc(strlen(program_name) + 1, sizeof(*program_name));
	if (!ipset->newargv[0]) {
		free(ipset);
		return NULL;
	}
	ipset_strlcpy(ipset->newargv[0], program_name,
		      strlen(program_name) + 1);
	ipset->newargc = 1;
	ipset->session = ipset_session_init(NULL, NULL);
	if (ipset->session == NULL) {
		free(ipset->newargv[0]);
		free(ipset);
		return NULL;
	}
	ipset_custom_printf(ipset, NULL, NULL, NULL, NULL);
	return ipset;
}

/**
 * ipset_fini - destroy an ipset library interface
 * @ipset: ipset structure
 *
 * Destroys an ipset library interface
 *
 * Returns 0 on success or a negative error code.
 */
int
ipset_fini(struct ipset *ipset)
{
	assert(ipset);

	if (ipset->session)
		ipset_session_fini(ipset->session);
	reset_argv(ipset);
	if (ipset->newargv[0])
		free(ipset->newargv[0]);

	free(ipset);
	return 0;
}

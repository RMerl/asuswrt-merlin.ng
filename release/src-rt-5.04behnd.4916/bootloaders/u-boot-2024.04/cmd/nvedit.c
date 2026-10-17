// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2013
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

/*
 * Support for persistent environment data
 *
 * The "environment" is stored on external storage as a list of '\0'
 * terminated "name=value" strings. The end of the list is marked by
 * a double '\0'. The environment is preceded by a 32 bit CRC over
 * the data part and, in case of redundant environment, a byte of
 * flags.
 *
 * This linearized representation will also be used before
 * relocation, i. e. as long as we don't have a full C runtime
 * environment. After that, we use a hash table.
 */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <env_internal.h>
#include <log.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/printk.h>
#include <u-boot/crc.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Maximum expected input data size for import command
 */
#define	MAX_ENV_SIZE	(1 << 20)	/* 1 MiB */

#ifndef CONFIG_SPL_BUILD
/*
 * Command interface: print one or all environment variables
 *
 * Returns 0 in case of error, or length of printed string
 */
static int env_print(char *name, int flag)
{
	char *res = NULL;
	ssize_t len;

	if (name) {		/* print a single name */
		struct env_entry e, *ep;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, ENV_FIND, &ep, &env_htab, flag);
		if (ep == NULL)
			return 0;
		len = printf("%s=%s\n", ep->key, ep->data);
		return len;
	}

	/* print whole list */
	len = hexport_r(&env_htab, '\n', flag, &res, 0, 0, NULL);

	if (len > 0) {
		puts(res);
		free(res);
		return len;
	}

	/* should never happen */
	printf("## Error: cannot export environment\n");
	return 0;
}

static int do_env_print(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int i;
	int rcode = 0;
	int env_flag = H_HIDE_DOT;

#if defined(CONFIG_CMD_NVEDIT_EFI)
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'e')
		return do_env_print_efi(cmdtp, flag, --argc, ++argv);
#endif

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'a') {
		argc--;
		argv++;
		env_flag &= ~H_HIDE_DOT;
	}

	if (argc == 1) {
		/* print all env vars */
		rcode = env_print(NULL, env_flag);
		if (!rcode)
			return 1;
		printf("\nEnvironment size: %d/%ld bytes\n",
			rcode, (ulong)ENV_SIZE);
		return 0;
	}

	/* print selected env vars */
	env_flag &= ~H_HIDE_DOT;
	for (i = 1; i < argc; ++i) {
		int rc = env_print(argv[i], env_flag);
		if (!rc) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			++rcode;
		}
	}

	return rcode;
}

#ifdef CONFIG_CMD_GREPENV
static int do_env_grep(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	char *res = NULL;
	int len, grep_how, grep_what;

	if (argc < 2)
		return CMD_RET_USAGE;

	grep_how  = H_MATCH_SUBSTR;	/* default: substring search	*/
	grep_what = H_MATCH_BOTH;	/* default: grep names and values */

	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;
		while (*++arg) {
			switch (*arg) {
#ifdef CONFIG_REGEX
			case 'e':		/* use regex matching */
				grep_how  = H_MATCH_REGEX;
				break;
#endif
			case 'n':		/* grep for name */
				grep_what = H_MATCH_KEY;
				break;
			case 'v':		/* grep for value */
				grep_what = H_MATCH_DATA;
				break;
			case 'b':		/* grep for both */
				grep_what = H_MATCH_BOTH;
				break;
			case '-':
				goto DONE;
			default:
				return CMD_RET_USAGE;
			}
		}
	}

DONE:
	len = hexport_r(&env_htab, '\n',
			flag | grep_what | grep_how,
			&res, 0, argc, argv);

	if (len > 0) {
		puts(res);
		free(res);
	}

	if (len < 2)
		return 1;

	return 0;
}
#endif
#endif /* CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
static int do_env_set(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	return env_do_env_set(flag, argc, argv, H_INTERACTIVE);
}

/*
 * Prompt for environment variable
 */
#if defined(CONFIG_CMD_ASKENV)
int do_env_ask(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char message[CONFIG_SYS_CBSIZE];
	int i, len, pos, size;
	char *local_args[4];
	char *endptr;

	local_args[0] = argv[0];
	local_args[1] = argv[1];
	local_args[2] = NULL;
	local_args[3] = NULL;

	/*
	 * Check the syntax:
	 *
	 * env_ask envname [message1 ...] [size]
	 */
	if (argc == 1)
		return CMD_RET_USAGE;

	/*
	 * We test the last argument if it can be converted
	 * into a decimal number.  If yes, we assume it's
	 * the size.  Otherwise we echo it as part of the
	 * message.
	 */
	i = dectoul(argv[argc - 1], &endptr);
	if (*endptr != '\0') {			/* no size */
		size = CONFIG_SYS_CBSIZE - 1;
	} else {				/* size given */
		size = i;
		--argc;
	}

	if (argc <= 2) {
		sprintf(message, "Please enter '%s': ", argv[1]);
	} else {
		/* env_ask envname message1 ... messagen [size] */
		for (i = 2, pos = 0; i < argc && pos+1 < sizeof(message); i++) {
			if (pos)
				message[pos++] = ' ';

			strncpy(message + pos, argv[i], sizeof(message) - pos);
			pos += strlen(argv[i]);
		}
		if (pos < sizeof(message) - 1) {
			message[pos++] = ' ';
			message[pos] = '\0';
		} else
			message[CONFIG_SYS_CBSIZE - 1] = '\0';
	}

	if (size >= CONFIG_SYS_CBSIZE)
		size = CONFIG_SYS_CBSIZE - 1;

	if (size <= 0)
		return 1;

	/* prompt for input */
	len = cli_readline(message);

	if (size < len)
		console_buffer[size] = '\0';

	len = 2;
	if (console_buffer[0] != '\0') {
		local_args[2] = console_buffer;
		len = 3;
	}

	/* Continue calling setenv code */
	return env_do_env_set(flag, len, local_args, H_INTERACTIVE);
}
#endif

#if defined(CONFIG_CMD_ENV_CALLBACK)
static int print_static_binding(const char *var_name, const char *callback_name,
				void *priv)
{
	printf("\t%-20s %-20s\n", var_name, callback_name);

	return 0;
}

static int print_active_callback(struct env_entry *entry)
{
	struct env_clbk_tbl *clbkp;
	int i;
	int num_callbacks;

	if (entry->callback == NULL)
		return 0;

	/* look up the callback in the linker-list */
	num_callbacks = ll_entry_count(struct env_clbk_tbl, env_clbk);
	for (i = 0, clbkp = ll_entry_start(struct env_clbk_tbl, env_clbk);
	     i < num_callbacks;
	     i++, clbkp++) {
		if (entry->callback == clbkp->callback)
			break;
	}

	if (i == num_callbacks)
		/* this should probably never happen, but just in case... */
		printf("\t%-20s %p\n", entry->key, entry->callback);
	else
		printf("\t%-20s %-20s\n", entry->key, clbkp->name);

	return 0;
}

/*
 * Print the callbacks available and what they are bound to
 */
int do_env_callback(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct env_clbk_tbl *clbkp;
	int i;
	int num_callbacks;

	/* Print the available callbacks */
	puts("Available callbacks:\n");
	puts("\tCallback Name\n");
	puts("\t-------------\n");
	num_callbacks = ll_entry_count(struct env_clbk_tbl, env_clbk);
	for (i = 0, clbkp = ll_entry_start(struct env_clbk_tbl, env_clbk);
	     i < num_callbacks;
	     i++, clbkp++)
		printf("\t%s\n", clbkp->name);
	puts("\n");

	/* Print the static bindings that may exist */
	puts("Static callback bindings:\n");
	printf("\t%-20s %-20s\n", "Variable Name", "Callback Name");
	printf("\t%-20s %-20s\n", "-------------", "-------------");
	env_attr_walk(ENV_CALLBACK_LIST_STATIC, print_static_binding, NULL);
	puts("\n");

	/* walk through each variable and print the callback if it has one */
	puts("Active callback bindings:\n");
	printf("\t%-20s %-20s\n", "Variable Name", "Callback Name");
	printf("\t%-20s %-20s\n", "-------------", "-------------");
	hwalk_r(&env_htab, print_active_callback);
	return 0;
}
#endif

#if defined(CONFIG_CMD_ENV_FLAGS)
static int print_static_flags(const char *var_name, const char *flags,
			      void *priv)
{
	enum env_flags_vartype type = env_flags_parse_vartype(flags);
	enum env_flags_varaccess access = env_flags_parse_varaccess(flags);

	printf("\t%-20s %-20s %-20s\n", var_name,
		env_flags_get_vartype_name(type),
		env_flags_get_varaccess_name(access));

	return 0;
}

static int print_active_flags(struct env_entry *entry)
{
	enum env_flags_vartype type;
	enum env_flags_varaccess access;

	if (entry->flags == 0)
		return 0;

	type = (enum env_flags_vartype)
		(entry->flags & ENV_FLAGS_VARTYPE_BIN_MASK);
	access = env_flags_parse_varaccess_from_binflags(entry->flags);
	printf("\t%-20s %-20s %-20s\n", entry->key,
		env_flags_get_vartype_name(type),
		env_flags_get_varaccess_name(access));

	return 0;
}

/*
 * Print the flags available and what variables have flags
 */
int do_env_flags(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	/* Print the available variable types */
	printf("Available variable type flags (position %d):\n",
		ENV_FLAGS_VARTYPE_LOC);
	puts("\tFlag\tVariable Type Name\n");
	puts("\t----\t------------------\n");
	env_flags_print_vartypes();
	puts("\n");

	/* Print the available variable access types */
	printf("Available variable access flags (position %d):\n",
		ENV_FLAGS_VARACCESS_LOC);
	puts("\tFlag\tVariable Access Name\n");
	puts("\t----\t--------------------\n");
	env_flags_print_varaccess();
	puts("\n");

	/* Print the static flags that may exist */
	puts("Static flags:\n");
	printf("\t%-20s %-20s %-20s\n", "Variable Name", "Variable Type",
		"Variable Access");
	printf("\t%-20s %-20s %-20s\n", "-------------", "-------------",
		"---------------");
	env_attr_walk(ENV_FLAGS_LIST_STATIC, print_static_flags, NULL);
	puts("\n");

	/* walk through each variable and print the flags if non-default */
	puts("Active flags:\n");
	printf("\t%-20s %-20s %-20s\n", "Variable Name", "Variable Type",
		"Variable Access");
	printf("\t%-20s %-20s %-20s\n", "-------------", "-------------",
		"---------------");
	hwalk_r(&env_htab, print_active_flags);
	return 0;
}
#endif

/*
 * Interactively edit an environment variable
 */
#if defined(CONFIG_CMD_EDITENV)
static int do_env_edit(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	char buffer[CONFIG_SYS_CBSIZE];
	char *init_val;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* before import into hashtable */
	if (!(gd->flags & GD_FLG_ENV_READY))
		return 1;

	/* Set read buffer to initial value or empty sting */
	init_val = env_get(argv[1]);
	if (init_val)
		snprintf(buffer, CONFIG_SYS_CBSIZE, "%s", init_val);
	else
		buffer[0] = '\0';

	if (cli_readline_into_buffer("edit: ", buffer, 0) < 0)
		return 1;

	if (buffer[0] == '\0') {
		const char * const _argv[3] = { "setenv", argv[1], NULL };

		return env_do_env_set(0, 2, (char * const *)_argv, H_INTERACTIVE);
	} else {
		const char * const _argv[4] = { "setenv", argv[1], buffer,
			NULL };

		return env_do_env_set(0, 3, (char * const *)_argv, H_INTERACTIVE);
	}
}
#endif /* CONFIG_CMD_EDITENV */

#if defined(CONFIG_CMD_SAVEENV) && !IS_ENABLED(CONFIG_ENV_IS_DEFAULT)
static int do_env_save(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	return env_save() ? 1 : 0;
}

U_BOOT_CMD(
	saveenv, 1, 0,	do_env_save,
	"save environment variables to persistent storage",
	""
);

#if defined(CONFIG_CMD_ERASEENV)
static int do_env_erase(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return env_erase() ? 1 : 0;
}

U_BOOT_CMD(
	eraseenv, 1, 0,	do_env_erase,
	"erase environment variables from persistent storage",
	""
);
#endif
#endif

#if defined(CONFIG_CMD_NVEDIT_LOAD)
static int do_env_load(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	return env_reload() ? 1 : 0;
}
#endif

#if defined(CONFIG_CMD_NVEDIT_SELECT)
static int do_env_select(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	return env_select(argv[1]) ? 1 : 0;
}
#endif

#endif /* CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
static int do_env_default(struct cmd_tbl *cmdtp, int flag,
			  int argc, char *const argv[])
{
	int all = 0, env_flag = H_INTERACTIVE;

	debug("Initial value for argc=%d\n", argc);
	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;

		while (*++arg) {
			switch (*arg) {
			case 'a':		/* default all */
				all = 1;
				break;
			case 'f':		/* force */
				env_flag |= H_FORCE;
				break;
			default:
				return cmd_usage(cmdtp);
			}
		}
	}
	debug("Final value for argc=%d\n", argc);
	if (all && (argc == 0)) {
		/* Reset the whole environment */
		env_set_default("## Resetting to default environment\n",
				env_flag);
		return 0;
	}
	if (!all && (argc > 0)) {
		/* Reset individual variables */
		env_set_default_vars(argc, argv, env_flag);
		return 0;
	}

	return cmd_usage(cmdtp);
}

static int do_env_delete(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	int env_flag = H_INTERACTIVE;
	int ret = 0;

	debug("Initial value for argc=%d\n", argc);
	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'f':		/* force */
				env_flag |= H_FORCE;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}
	debug("Final value for argc=%d\n", argc);

	env_inc_id();

	while (--argc > 0) {
		char *name = *++argv;

		if (hdelete_r(name, &env_htab, env_flag))
			ret = 1;
	}

	return ret;
}

#ifdef CONFIG_CMD_EXPORTENV
/*
 * env export [-t | -b | -c] [-s size] addr [var ...]
 *	-t:	export as text format; if size is given, data will be
 *		padded with '\0' bytes; if not, one terminating '\0'
 *		will be added (which is included in the "filesize"
 *		setting so you can for exmple copy this to flash and
 *		keep the termination).
 *	-b:	export as binary format (name=value pairs separated by
 *		'\0', list end marked by double "\0\0")
 *	-c:	export as checksum protected environment format as
 *		used for example by "saveenv" command
 *	-s size:
 *		size of output buffer
 *	addr:	memory address where environment gets stored
 *	var...	List of variable names that get included into the
 *		export. Without arguments, the whole environment gets
 *		exported.
 *
 * With "-c" and size is NOT given, then the export command will
 * format the data as currently used for the persistent storage,
 * i. e. it will use CONFIG_ENV_SECT_SIZE as output block size and
 * prepend a valid CRC32 checksum and, in case of redundant
 * environment, a "current" redundancy flag. If size is given, this
 * value will be used instead of CONFIG_ENV_SECT_SIZE; again, CRC32
 * checksum and redundancy flag will be inserted.
 *
 * With "-b" and "-t", always only the real data (including a
 * terminating '\0' byte) will be written; here the optional size
 * argument will be used to make sure not to overflow the user
 * provided buffer; the command will abort if the size is not
 * sufficient. Any remaining space will be '\0' padded.
 *
 * On successful return, the variable "filesize" will be set.
 * Note that filesize includes the trailing/terminating '\0' byte(s).
 *
 * Usage scenario:  create a text snapshot/backup of the current settings:
 *
 *	=> env export -t 100000
 *	=> era ${backup_addr} +${filesize}
 *	=> cp.b 100000 ${backup_addr} ${filesize}
 *
 * Re-import this snapshot, deleting all other settings:
 *
 *	=> env import -d -t ${backup_addr}
 */
static int do_env_export(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	char	buf[32];
	ulong	addr;
	char	*ptr, *cmd, *res;
	size_t	size = 0;
	ssize_t	len;
	env_t	*envp;
	char	sep = '\n';
	int	chk = 0;
	int	fmt = 0;

	cmd = *argv;

	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;
		while (*++arg) {
			switch (*arg) {
			case 'b':		/* raw binary format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				break;
			case 'c':		/* external checksum format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				chk = 1;
				break;
			case 's':		/* size given */
				if (--argc <= 0)
					return cmd_usage(cmdtp);
				size = hextoul(*++argv, NULL);
				goto NXTARG;
			case 't':		/* text format */
				if (fmt++)
					goto sep_err;
				sep = '\n';
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
NXTARG:		;
	}

	if (argc < 1)
		return CMD_RET_USAGE;

	addr = hextoul(argv[0], NULL);
	ptr = map_sysmem(addr, size);

	if (size)
		memset(ptr, '\0', size);

	argc--;
	argv++;

	if (sep) {		/* export as text file */
		len = hexport_r(&env_htab, sep,
				H_MATCH_KEY | H_MATCH_IDENT,
				&ptr, size, argc, argv);
		if (len < 0) {
			pr_err("## Error: Cannot export environment: errno = %d\n",
			       errno);
			return 1;
		}
		sprintf(buf, "%zX", (size_t)len);
		env_set("filesize", buf);

		return 0;
	}

	envp = (env_t *)ptr;

	if (chk)		/* export as checksum protected block */
		res = (char *)envp->data;
	else			/* export as raw binary data */
		res = ptr;

	len = hexport_r(&env_htab, '\0',
			H_MATCH_KEY | H_MATCH_IDENT,
			&res, ENV_SIZE, argc, argv);
	if (len < 0) {
		pr_err("## Error: Cannot export environment: errno = %d\n",
		       errno);
		return 1;
	}

	if (chk) {
		envp->crc = crc32(0, envp->data,
				size ? size - offsetof(env_t, data) : ENV_SIZE);
#ifdef CONFIG_ENV_ADDR_REDUND
		envp->flags = ENV_REDUND_ACTIVE;
#endif
	}
	env_set_hex("filesize", len + offsetof(env_t, data));

	return 0;

sep_err:
	printf("## Error: %s: only one of \"-b\", \"-c\" or \"-t\" allowed\n",
	       cmd);
	return 1;
}
#endif

#ifdef CONFIG_CMD_IMPORTENV
/*
 * env import [-d] [-t [-r] | -b | -c] addr [size] [var ...]
 *	-d:	delete existing environment before importing if no var is
 *		passed; if vars are passed, if one var is in the current
 *		environment but not in the environment at addr, delete var from
 *		current environment;
 *		otherwise overwrite / append to existing definitions
 *	-t:	assume text format; either "size" must be given or the
 *		text data must be '\0' terminated
 *	-r:	handle CRLF like LF, that means exported variables with
 *		a content which ends with \r won't get imported. Used
 *		to import text files created with editors which are using CRLF
 *		for line endings. Only effective in addition to -t.
 *	-b:	assume binary format ('\0' separated, "\0\0" terminated)
 *	-c:	assume checksum protected environment format
 *	addr:	memory address to read from
 *	size:	length of input data; if missing, proper '\0'
 *		termination is mandatory
 *		if var is set and size should be missing (i.e. '\0'
 *		termination), set size to '-'
 *	var...	List of the names of the only variables that get imported from
 *		the environment at address 'addr'. Without arguments, the whole
 *		environment gets imported.
 */
static int do_env_import(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	ulong	addr;
	char	*cmd, *ptr;
	char	sep = '\n';
	int	chk = 0;
	int	fmt = 0;
	int	del = 0;
	int	crlf_is_lf = 0;
	int	wl = 0;
	size_t	size;

	cmd = *argv;

	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;
		while (*++arg) {
			switch (*arg) {
			case 'b':		/* raw binary format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				break;
			case 'c':		/* external checksum format */
				if (fmt++)
					goto sep_err;
				sep = '\0';
				chk = 1;
				break;
			case 't':		/* text format */
				if (fmt++)
					goto sep_err;
				sep = '\n';
				break;
			case 'r':		/* handle CRLF like LF */
				crlf_is_lf = 1;
				break;
			case 'd':
				del = 1;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}

	if (argc < 1)
		return CMD_RET_USAGE;

	if (!fmt)
		printf("## Warning: defaulting to text format\n");

	if (sep != '\n' && crlf_is_lf )
		crlf_is_lf = 0;

	addr = hextoul(argv[0], NULL);
	ptr = map_sysmem(addr, 0);

	if (argc >= 2 && strcmp(argv[1], "-")) {
		size = hextoul(argv[1], NULL);
	} else if (chk) {
		puts("## Error: external checksum format must pass size\n");
		return CMD_RET_FAILURE;
	} else {
		char *s = ptr;

		size = 0;

		while (size < MAX_ENV_SIZE) {
			if ((*s == sep) && (*(s+1) == '\0'))
				break;
			++s;
			++size;
		}
		if (size == MAX_ENV_SIZE) {
			printf("## Warning: Input data exceeds %d bytes"
				" - truncated\n", MAX_ENV_SIZE);
		}
		size += 2;
		printf("## Info: input data size = %zu = 0x%zX\n", size, size);
	}

	if (argc > 2)
		wl = 1;

	if (chk) {
		uint32_t crc;
		env_t *ep = (env_t *)ptr;

		if (size <= offsetof(env_t, data)) {
			printf("## Error: Invalid size 0x%zX\n", size);
			return 1;
		}

		size -= offsetof(env_t, data);
		memcpy(&crc, &ep->crc, sizeof(crc));

		if (crc32(0, ep->data, size) != crc) {
			puts("## Error: bad CRC, import failed\n");
			return 1;
		}
		ptr = (char *)ep->data;
	}

	if (!himport_r(&env_htab, ptr, size, sep, del ? 0 : H_NOCLEAR,
		       crlf_is_lf, wl ? argc - 2 : 0, wl ? &argv[2] : NULL)) {
		pr_err("## Error: Environment import failed: errno = %d\n",
		       errno);
		return 1;
	}
	gd->flags |= GD_FLG_ENV_READY;

	return 0;

sep_err:
	printf("## %s: only one of \"-b\", \"-c\" or \"-t\" allowed\n",
		cmd);
	return 1;
}
#endif

#if defined(CONFIG_CMD_NVEDIT_INDIRECT)
static int do_env_indirect(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	char *to = argv[1];
	char *from = argv[2];
	char *default_value = NULL;
	int ret = 0;
	char *val;

	if (argc < 3 || argc > 4) {
		return CMD_RET_USAGE;
	}

	if (argc == 4) {
		default_value = argv[3];
	}

	val = env_get(from) ?: default_value;
	if (!val) {
		printf("## env indirect: Environment variable for <from> (%s) does not exist.\n", from);

		return CMD_RET_FAILURE;
	}

	ret = env_set(to, val);

	if (ret == 0) {
		return CMD_RET_SUCCESS;
	}
	else {
		return CMD_RET_FAILURE;
	}
}
#endif

#if defined(CONFIG_CMD_NVEDIT_INFO)
/*
 * print_env_info - print environment information
 */
static int print_env_info(void)
{
	const char *value;

	/* print environment validity value */
	switch (gd->env_valid) {
	case ENV_INVALID:
		value = "invalid";
		break;
	case ENV_VALID:
		value = "valid";
		break;
	case ENV_REDUND:
		value = "redundant";
		break;
	default:
		value = "unknown";
		break;
	}
	printf("env_valid = %s\n", value);

	/* print environment ready flag */
	value = gd->flags & GD_FLG_ENV_READY ? "true" : "false";
	printf("env_ready = %s\n", value);

	/* print environment using default flag */
	value = gd->flags & GD_FLG_ENV_DEFAULT ? "true" : "false";
	printf("env_use_default = %s\n", value);

	return CMD_RET_SUCCESS;
}

#define ENV_INFO_IS_DEFAULT	BIT(0) /* default environment bit mask */
#define ENV_INFO_IS_PERSISTED	BIT(1) /* environment persistence bit mask */

/*
 * env info - display environment information
 * env info [-d] - evaluate whether default environment is used
 * env info [-p] - evaluate whether environment can be persisted
 *      Add [-q] - quiet mode, use only for command result, for test by example:
 *                 test env info -p -d -q
 */
static int do_env_info(struct cmd_tbl *cmdtp, int flag,
		       int argc, char *const argv[])
{
	int eval_flags = 0;
	int eval_results = 0;
	bool quiet = false;
#if defined(CONFIG_CMD_SAVEENV) && !IS_ENABLED(CONFIG_ENV_IS_DEFAULT)
	enum env_location loc;
#endif

	/* display environment information */
	if (argc <= 1)
		return print_env_info();

	/* process options */
	while (--argc > 0 && **++argv == '-') {
		char *arg = *argv;

		while (*++arg) {
			switch (*arg) {
			case 'd':
				eval_flags |= ENV_INFO_IS_DEFAULT;
				break;
			case 'p':
				eval_flags |= ENV_INFO_IS_PERSISTED;
				break;
			case 'q':
				quiet = true;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}

	/* evaluate whether default environment is used */
	if (eval_flags & ENV_INFO_IS_DEFAULT) {
		if (gd->flags & GD_FLG_ENV_DEFAULT) {
			if (!quiet)
				printf("Default environment is used\n");
			eval_results |= ENV_INFO_IS_DEFAULT;
		} else {
			if (!quiet)
				printf("Environment was loaded from persistent storage\n");
		}
	}

	/* evaluate whether environment can be persisted */
	if (eval_flags & ENV_INFO_IS_PERSISTED) {
#if defined(CONFIG_CMD_SAVEENV) && !IS_ENABLED(CONFIG_ENV_IS_DEFAULT)
		loc = env_get_location(ENVOP_SAVE, gd->env_load_prio);
		if (ENVL_NOWHERE != loc && ENVL_UNKNOWN != loc) {
			if (!quiet)
				printf("Environment can be persisted\n");
			eval_results |= ENV_INFO_IS_PERSISTED;
		} else {
			if (!quiet)
				printf("Environment cannot be persisted\n");
		}
#else
		if (!quiet)
			printf("Environment cannot be persisted\n");
#endif
	}

	/* The result of evaluations is combined with AND */
	if (eval_flags != eval_results)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
#endif

#if defined(CONFIG_CMD_ENV_EXISTS)
static int do_env_exists(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct env_entry e, *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	e.key = argv[1];
	e.data = NULL;
	hsearch_r(e, ENV_FIND, &ep, &env_htab, 0);

	return (ep == NULL) ? 1 : 0;
}
#endif

/*
 * New command line interface: "env" command with subcommands
 */
static struct cmd_tbl cmd_env_sub[] = {
#if defined(CONFIG_CMD_ASKENV)
	U_BOOT_CMD_MKENT(ask, CONFIG_SYS_MAXARGS, 1, do_env_ask, "", ""),
#endif
	U_BOOT_CMD_MKENT(default, 1, 0, do_env_default, "", ""),
	U_BOOT_CMD_MKENT(delete, CONFIG_SYS_MAXARGS, 0, do_env_delete, "", ""),
#if defined(CONFIG_CMD_EDITENV)
	U_BOOT_CMD_MKENT(edit, 2, 0, do_env_edit, "", ""),
#endif
#if defined(CONFIG_CMD_ENV_CALLBACK)
	U_BOOT_CMD_MKENT(callbacks, 1, 0, do_env_callback, "", ""),
#endif
#if defined(CONFIG_CMD_ENV_FLAGS)
	U_BOOT_CMD_MKENT(flags, 1, 0, do_env_flags, "", ""),
#endif
#if defined(CONFIG_CMD_EXPORTENV)
	U_BOOT_CMD_MKENT(export, 4, 0, do_env_export, "", ""),
#endif
#if defined(CONFIG_CMD_GREPENV)
	U_BOOT_CMD_MKENT(grep, CONFIG_SYS_MAXARGS, 1, do_env_grep, "", ""),
#endif
#if defined(CONFIG_CMD_IMPORTENV)
	U_BOOT_CMD_MKENT(import, 5, 0, do_env_import, "", ""),
#endif
#if defined(CONFIG_CMD_NVEDIT_INDIRECT)
	U_BOOT_CMD_MKENT(indirect, 3, 0, do_env_indirect, "", ""),
#endif
#if defined(CONFIG_CMD_NVEDIT_INFO)
	U_BOOT_CMD_MKENT(info, 3, 0, do_env_info, "", ""),
#endif
#if defined(CONFIG_CMD_NVEDIT_LOAD)
	U_BOOT_CMD_MKENT(load, 1, 0, do_env_load, "", ""),
#endif
	U_BOOT_CMD_MKENT(print, CONFIG_SYS_MAXARGS, 1, do_env_print, "", ""),
#if defined(CONFIG_CMD_RUN)
	U_BOOT_CMD_MKENT(run, CONFIG_SYS_MAXARGS, 1, do_run, "", ""),
#endif
#if defined(CONFIG_CMD_SAVEENV) && !IS_ENABLED(CONFIG_ENV_IS_DEFAULT)
	U_BOOT_CMD_MKENT(save, 1, 0, do_env_save, "", ""),
#if defined(CONFIG_CMD_ERASEENV)
	U_BOOT_CMD_MKENT(erase, 1, 0, do_env_erase, "", ""),
#endif
#endif
#if defined(CONFIG_CMD_NVEDIT_SELECT)
	U_BOOT_CMD_MKENT(select, 2, 0, do_env_select, "", ""),
#endif
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 0, do_env_set, "", ""),
#if defined(CONFIG_CMD_ENV_EXISTS)
	U_BOOT_CMD_MKENT(exists, 2, 0, do_env_exists, "", ""),
#endif
};

static int do_env(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "env" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_env_sub, ARRAY_SIZE(cmd_env_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(env,
#if defined(CONFIG_CMD_ASKENV)
	"ask name [message] [size] - ask for environment variable\nenv "
#endif
#if defined(CONFIG_CMD_ENV_CALLBACK)
	"callbacks - print callbacks and their associated variables\nenv "
#endif
	"default [-f] -a - [forcibly] reset default environment\n"
	"env default [-f] var [...] - [forcibly] reset variable(s) to their default values\n"
	"env delete [-f] var [...] - [forcibly] delete variable(s)\n"
#if defined(CONFIG_CMD_EDITENV)
	"env edit name - edit environment variable\n"
#endif
#if defined(CONFIG_CMD_ENV_EXISTS)
	"env exists name - tests for existence of variable\n"
#endif
#if defined(CONFIG_CMD_EXPORTENV)
	"env export [-t | -b | -c] [-s size] addr [var ...] - export environment\n"
#endif
#if defined(CONFIG_CMD_ENV_FLAGS)
	"env flags - print variables that have non-default flags\n"
#endif
#if defined(CONFIG_CMD_GREPENV)
#ifdef CONFIG_REGEX
	"env grep [-e] [-n | -v | -b] string [...] - search environment\n"
#else
	"env grep [-n | -v | -b] string [...] - search environment\n"
#endif
#endif
#if defined(CONFIG_CMD_IMPORTENV)
	"env import [-d] [-t [-r] | -b | -c] addr [size] [var ...] - import environment\n"
#endif
#if defined(CONFIG_CMD_NVEDIT_INDIRECT)
	"env indirect <to> <from> [default] - sets <to> to the value of <from>, using [default] when unset\n"
#endif
#if defined(CONFIG_CMD_NVEDIT_INFO)
	"env info - display environment information\n"
	"env info [-d] [-p] [-q] - evaluate environment information\n"
	"      \"-d\": default environment is used\n"
	"      \"-p\": environment can be persisted\n"
	"      \"-q\": quiet output\n"
#endif
	"env print [-a | name ...] - print environment\n"
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"env print -e [-guid guid] [-n] [name ...] - print UEFI environment\n"
#endif
#if defined(CONFIG_CMD_RUN)
	"env run var [...] - run commands in an environment variable\n"
#endif
#if defined(CONFIG_CMD_SAVEENV) && !IS_ENABLED(CONFIG_ENV_IS_DEFAULT)
	"env save - save environment\n"
#if defined(CONFIG_CMD_ERASEENV)
	"env erase - erase environment\n"
#endif
#endif
#if defined(CONFIG_CMD_NVEDIT_LOAD)
	"env load - load environment\n"
#endif
#if defined(CONFIG_CMD_NVEDIT_SELECT)
	"env select [target] - select environment target\n"
#endif
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"env set -e [-nv][-bs][-rt][-at][-a][-i addr:size][-v] name [arg ...]\n"
	"    - set UEFI variable; unset if '-i' or 'arg' not specified\n"
#endif
	"env set [-f] name [arg ...]\n");

U_BOOT_CMD(
	env, CONFIG_SYS_MAXARGS, 1, do_env,
	"environment handling commands", env_help_text
);

/*
 * Old command line interface, kept for compatibility
 */

#if defined(CONFIG_CMD_EDITENV)
U_BOOT_CMD_COMPLETE(
	editenv, 2, 0,	do_env_edit,
	"edit environment variable",
	"name\n"
	"    - edit environment variable 'name'",
	var_complete
);
#endif

U_BOOT_CMD_COMPLETE(
	printenv, CONFIG_SYS_MAXARGS, 1,	do_env_print,
	"print environment variables",
	"[-a]\n    - print [all] values of all environment variables\n"
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"printenv -e [-guid guid][-n] [name ...]\n"
	"    - print UEFI variable 'name' or all the variables\n"
	"      \"-guid\": GUID xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\n"
	"      \"-n\": suppress dumping variable's value\n"
#endif
	"printenv name ...\n"
	"    - print value of environment variable 'name'",
	var_complete
);

#ifdef CONFIG_CMD_GREPENV
U_BOOT_CMD_COMPLETE(
	grepenv, CONFIG_SYS_MAXARGS, 0,  do_env_grep,
	"search environment variables",
#ifdef CONFIG_REGEX
	"[-e] [-n | -v | -b] string ...\n"
#else
	"[-n | -v | -b] string ...\n"
#endif
	"    - list environment name=value pairs matching 'string'\n"
#ifdef CONFIG_REGEX
	"      \"-e\": enable regular expressions;\n"
#endif
	"      \"-n\": search variable names; \"-v\": search values;\n"
	"      \"-b\": search both names and values (default)",
	var_complete
);
#endif

U_BOOT_CMD_COMPLETE(
	setenv, CONFIG_SYS_MAXARGS, 0,	do_env_set,
	"set environment variables",
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"-e [-guid guid][-nv][-bs][-rt][-at][-a][-v]\n"
	"        [-i addr:size name], or [name [value ...]]\n"
	"    - set UEFI variable 'name' to 'value' ...'\n"
	"      \"-guid\": GUID xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\n"
	"      \"-nv\": set non-volatile attribute\n"
	"      \"-bs\": set boot-service attribute\n"
	"      \"-rt\": set runtime attribute\n"
	"      \"-at\": set time-based authentication attribute\n"
	"      \"-a\": append-write\n"
	"      \"-i addr,size\": use <addr,size> as variable's value\n"
	"      \"-v\": verbose message\n"
	"    - delete UEFI variable 'name' if 'value' not specified\n"
#endif
	"setenv [-f] name value ...\n"
	"    - [forcibly] set environment variable 'name' to 'value ...'\n"
	"setenv [-f] name\n"
	"    - [forcibly] delete environment variable 'name'",
	var_complete
);

#if defined(CONFIG_CMD_ASKENV)

U_BOOT_CMD(
	askenv,	CONFIG_SYS_MAXARGS,	1,	do_env_ask,
	"get environment variables from stdin",
	"name [message] [size]\n"
	"    - get environment variable 'name' from stdin (max 'size' chars)"
);
#endif

#if defined(CONFIG_CMD_RUN)
U_BOOT_CMD_COMPLETE(
	run,	CONFIG_SYS_MAXARGS,	1,	do_run,
	"run commands in an environment variable",
	"var [...]\n"
	"    - run the commands in the environment variable(s) 'var'",
	var_complete
);
#endif
#endif /* CONFIG_SPL_BUILD */

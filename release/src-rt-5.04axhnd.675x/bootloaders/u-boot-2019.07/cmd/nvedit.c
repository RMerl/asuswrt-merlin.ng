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
#include <environment.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <watchdog.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if	defined(CONFIG_ENV_IS_IN_EEPROM)	|| \
	defined(CONFIG_ENV_IS_IN_FLASH)		|| \
	defined(CONFIG_ENV_IS_IN_MMC)		|| \
	defined(CONFIG_ENV_IS_IN_FAT)		|| \
	defined(CONFIG_ENV_IS_IN_EXT4)		|| \
	defined(CONFIG_ENV_IS_IN_NAND)		|| \
	defined(CONFIG_ENV_IS_IN_NVRAM)		|| \
	defined(CONFIG_ENV_IS_IN_ONENAND)	|| \
	defined(CONFIG_ENV_IS_IN_SATA)		|| \
	defined(CONFIG_ENV_IS_IN_SPI_FLASH)	|| \
	defined(CONFIG_ENV_IS_IN_REMOTE)	|| \
	defined(CONFIG_ENV_IS_IN_BOOT_MAGIC)	|| \
	defined(CONFIG_ENV_IS_IN_UBI)

#define ENV_IS_IN_DEVICE

#endif

#if	!defined(ENV_IS_IN_DEVICE)		&& \
	!defined(CONFIG_ENV_IS_NOWHERE)
# error Define one of CONFIG_ENV_IS_IN_{EEPROM|FLASH|MMC|FAT|EXT4|\
NAND|NVRAM|ONENAND|SATA|SPI_FLASH|REMOTE|BOOT_MAGIC|UBI} or CONFIG_ENV_IS_NOWHERE
#endif

/*
 * Maximum expected input data size for import command
 */
#define	MAX_ENV_SIZE	(1 << 20)	/* 1 MiB */

/*
 * This variable is incremented on each do_env_set(), so it can
 * be used via get_env_id() as an indication, if the environment
 * has changed or not. So it is possible to reread an environment
 * variable only if the environment was changed ... done so for
 * example in NetInitLoop()
 */
static int env_id = 1;

int get_env_id(void)
{
	return env_id;
}

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
		ENTRY e, *ep;

		e.key = name;
		e.data = NULL;
		hsearch_r(e, FIND, &ep, &env_htab, flag);
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

static int do_env_print(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
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
static int do_env_grep(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
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

/*
 * Set a new environment variable,
 * or replace or delete an existing one.
 */
static int _do_env_set(int flag, int argc, char * const argv[], int env_flag)
{
	int   i, len;
	char  *name, *value, *s;
	ENTRY e, *ep;

	debug("Initial value for argc=%d\n", argc);

#if CONFIG_IS_ENABLED(CMD_NVEDIT_EFI)
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'e')
		return do_env_set_efi(NULL, flag, --argc, ++argv);
#endif

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
	name = argv[1];

	if (strchr(name, '=')) {
		printf("## Error: illegal character '='"
		       "in variable name \"%s\"\n", name);
		return 1;
	}

	env_id++;

	/* Delete only ? */
	if (argc < 3 || argv[2] == NULL) {
		int rc = hdelete_r(name, &env_htab, env_flag);
		return !rc;
	}

	/*
	 * Insert / replace new value
	 */
	for (i = 2, len = 0; i < argc; ++i)
		len += strlen(argv[i]) + 1;

	value = malloc(len);
	if (value == NULL) {
		printf("## Can't malloc %d bytes\n", len);
		return 1;
	}
	for (i = 2, s = value; i < argc; ++i) {
		char *v = argv[i];

		while ((*s++ = *v++) != '\0')
			;
		*(s - 1) = ' ';
	}
	if (s != value)
		*--s = '\0';

	e.key	= name;
	e.data	= value;
	hsearch_r(e, ENTER, &ep, &env_htab, env_flag);
	free(value);
	if (!ep) {
		printf("## Error inserting \"%s\" variable, errno=%d\n",
			name, errno);
		return 1;
	}

	return 0;
}

int env_set(const char *varname, const char *varvalue)
{
	const char * const argv[4] = { "setenv", varname, varvalue, NULL };

	/* before import into hashtable */
	if (!(gd->flags & GD_FLG_ENV_READY))
		return 1;

	if (varvalue == NULL || varvalue[0] == '\0')
		return _do_env_set(0, 2, (char * const *)argv, H_PROGRAMMATIC);
	else
		return _do_env_set(0, 3, (char * const *)argv, H_PROGRAMMATIC);
}

/**
 * Set an environment variable to an integer value
 *
 * @param varname	Environment variable to set
 * @param value		Value to set it to
 * @return 0 if ok, 1 on error
 */
int env_set_ulong(const char *varname, ulong value)
{
	/* TODO: this should be unsigned */
	char *str = simple_itoa(value);

	return env_set(varname, str);
}

/**
 * Set an environment variable to an value in hex
 *
 * @param varname	Environment variable to set
 * @param value		Value to set it to
 * @return 0 if ok, 1 on error
 */
int env_set_hex(const char *varname, ulong value)
{
	char str[17];

	sprintf(str, "%lx", value);
	return env_set(varname, str);
}

ulong env_get_hex(const char *varname, ulong default_val)
{
	const char *s;
	ulong value;
	char *endp;

	s = env_get(varname);
	if (s)
		value = simple_strtoul(s, &endp, 16);
	if (!s || endp == s)
		return default_val;

	return value;
}

void eth_parse_enetaddr(const char *addr, uint8_t *enetaddr)
{
	char *end;
	int i;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

int eth_env_get_enetaddr(const char *name, uint8_t *enetaddr)
{
	eth_parse_enetaddr(env_get(name), enetaddr);
	return is_valid_ethaddr(enetaddr);
}

int eth_env_set_enetaddr(const char *name, const uint8_t *enetaddr)
{
	char buf[ARP_HLEN_ASCII + 1];

	if (eth_env_get_enetaddr(name, (uint8_t *)buf))
		return -EEXIST;

	sprintf(buf, "%pM", enetaddr);

	return env_set(name, buf);
}

#ifndef CONFIG_SPL_BUILD
static int do_env_set(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	return _do_env_set(flag, argc, argv, H_INTERACTIVE);
}

/*
 * Prompt for environment variable
 */
#if defined(CONFIG_CMD_ASKENV)
int do_env_ask(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
	i = simple_strtoul(argv[argc - 1], &endptr, 10);
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
	return _do_env_set(flag, len, local_args, H_INTERACTIVE);
}
#endif

#if defined(CONFIG_CMD_ENV_CALLBACK)
static int print_static_binding(const char *var_name, const char *callback_name,
				void *priv)
{
	printf("\t%-20s %-20s\n", var_name, callback_name);

	return 0;
}

static int print_active_callback(ENTRY *entry)
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
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
		if (entry->callback == clbkp->callback + gd->reloc_off)
#else
		if (entry->callback == clbkp->callback)
#endif
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
int do_env_callback(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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

static int print_active_flags(ENTRY *entry)
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
int do_env_flags(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
static int do_env_edit(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
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

		return _do_env_set(0, 2, (char * const *)_argv, H_INTERACTIVE);
	} else {
		const char * const _argv[4] = { "setenv", argv[1], buffer,
			NULL };

		return _do_env_set(0, 3, (char * const *)_argv, H_INTERACTIVE);
	}
}
#endif /* CONFIG_CMD_EDITENV */
#endif /* CONFIG_SPL_BUILD */

/*
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */
char *env_get(const char *name)
{
	if (gd->flags & GD_FLG_ENV_READY) { /* after import into hashtable */
		ENTRY e, *ep;

		WATCHDOG_RESET();

		e.key	= name;
		e.data	= NULL;
		hsearch_r(e, FIND, &ep, &env_htab, 0);

		return ep ? ep->data : NULL;
	}

	/* restricted capabilities before import */
	if (env_get_f(name, (char *)(gd->env_buf), sizeof(gd->env_buf)) > 0)
		return (char *)(gd->env_buf);

	return NULL;
}

/*
 * Look up variable from environment for restricted C runtime env.
 */
int env_get_f(const char *name, char *buf, unsigned len)
{
	int i, nxt, c;

	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		int val, n;

		for (nxt = i; (c = env_get_char(nxt)) != '\0'; ++nxt) {
			if (c < 0)
				return c;
			if (nxt >= CONFIG_ENV_SIZE)
				return -1;
		}

		val = envmatch((uchar *)name, i);
		if (val < 0)
			continue;

		/* found; copy out */
		for (n = 0; n < len; ++n, ++buf) {
			c = env_get_char(val++);
			if (c < 0)
				return c;
			*buf = c;
			if (*buf == '\0')
				return n;
		}

		if (n)
			*--buf = '\0';

		printf("env_buf [%u bytes] too small for value of \"%s\"\n",
		       len, name);

		return n;
	}

	return -1;
}

/**
 * Decode the integer value of an environment variable and return it.
 *
 * @param name		Name of environment variable
 * @param base		Number base to use (normally 10, or 16 for hex)
 * @param default_val	Default value to return if the variable is not
 *			found
 * @return the decoded value, or default_val if not found
 */
ulong env_get_ulong(const char *name, int base, ulong default_val)
{
	/*
	 * We can use env_get() here, even before relocation, since the
	 * environment variable value is an integer and thus short.
	 */
	const char *str = env_get(name);

	return str ? simple_strtoul(str, NULL, base) : default_val;
}

#ifndef CONFIG_SPL_BUILD
#if defined(CONFIG_CMD_SAVEENV) && defined(ENV_IS_IN_DEVICE)
static int do_env_save(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	return env_save() ? 1 : 0;
}

U_BOOT_CMD(
	saveenv, 1, 0,	do_env_save,
	"save environment variables to persistent storage",
	""
);
#endif
#endif /* CONFIG_SPL_BUILD */


/*
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else -1.
 */
int envmatch(uchar *s1, int i2)
{
	if (s1 == NULL)
		return -1;

	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return i2;

	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return i2;

	return -1;
}

#ifndef CONFIG_SPL_BUILD
static int do_env_default(cmd_tbl_t *cmdtp, int flag,
			  int argc, char * const argv[])
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
		set_default_env("## Resetting to default environment\n",
				env_flag);
		return 0;
	}
	if (!all && (argc > 0)) {
		/* Reset individual variables */
		set_default_vars(argc, argv, env_flag);
		return 0;
	}

	return cmd_usage(cmdtp);
}

static int do_env_delete(cmd_tbl_t *cmdtp, int flag,
			 int argc, char * const argv[])
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

	env_id++;

	while (--argc > 0) {
		char *name = *++argv;

		if (!hdelete_r(name, &env_htab, env_flag))
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
static int do_env_export(cmd_tbl_t *cmdtp, int flag,
			 int argc, char * const argv[])
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
				size = simple_strtoul(*++argv, NULL, 16);
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

	addr = simple_strtoul(argv[0], NULL, 16);
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
		envp->flags = ACTIVE_FLAG;
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
static int do_env_import(cmd_tbl_t *cmdtp, int flag,
			 int argc, char * const argv[])
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

	addr = simple_strtoul(argv[0], NULL, 16);
	ptr = map_sysmem(addr, 0);

	if (argc >= 2 && strcmp(argv[1], "-")) {
		size = simple_strtoul(argv[1], NULL, 16);
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

#if defined(CONFIG_CMD_ENV_EXISTS)
static int do_env_exists(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	ENTRY e, *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	e.key = argv[1];
	e.data = NULL;
	hsearch_r(e, FIND, &ep, &env_htab, 0);

	return (ep == NULL) ? 1 : 0;
}
#endif

/*
 * New command line interface: "env" command with subcommands
 */
static cmd_tbl_t cmd_env_sub[] = {
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
	U_BOOT_CMD_MKENT(print, CONFIG_SYS_MAXARGS, 1, do_env_print, "", ""),
#if defined(CONFIG_CMD_RUN)
	U_BOOT_CMD_MKENT(run, CONFIG_SYS_MAXARGS, 1, do_run, "", ""),
#endif
#if defined(CONFIG_CMD_SAVEENV) && defined(ENV_IS_IN_DEVICE)
	U_BOOT_CMD_MKENT(save, 1, 0, do_env_save, "", ""),
#endif
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 0, do_env_set, "", ""),
#if defined(CONFIG_CMD_ENV_EXISTS)
	U_BOOT_CMD_MKENT(exists, 2, 0, do_env_exists, "", ""),
#endif
};

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void env_reloc(void)
{
	fixup_cmdtable(cmd_env_sub, ARRAY_SIZE(cmd_env_sub));
}
#endif

static int do_env(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

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

#ifdef CONFIG_SYS_LONGHELP
static char env_help_text[] =
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
	"env print [-a | name ...] - print environment\n"
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"env print -e [name ...] - print UEFI environment\n"
#endif
#if defined(CONFIG_CMD_RUN)
	"env run var [...] - run commands in an environment variable\n"
#endif
#if defined(CONFIG_CMD_SAVEENV) && defined(ENV_IS_IN_DEVICE)
	"env save - save environment\n"
#endif
#if defined(CONFIG_CMD_NVEDIT_EFI)
	"env set -e name [arg ...] - set UEFI variable; unset if 'arg' not specified\n"
#endif
	"env set [-f] name [arg ...]\n";
#endif

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
	"printenv -e [name ...]\n"
	"    - print UEFI variable 'name' or all the variables\n"
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
	"-e [-nv] name [value ...]\n"
	"    - set UEFI variable 'name' to 'value' ...'\n"
	"      'nv' option makes the variable non-volatile\n"
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

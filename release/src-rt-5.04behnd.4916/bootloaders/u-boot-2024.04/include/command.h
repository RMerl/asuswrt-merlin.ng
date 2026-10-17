/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Definitions for Command Processor
 */
#ifndef __COMMAND_H
#define __COMMAND_H

#include <env.h>
#include <linker_lists.h>

#include <linux/compiler_attributes.h>

#ifndef NULL
#define NULL	0
#endif

/* Default to a width of 8 characters for help message command width */
#ifndef CFG_SYS_HELP_CMD_WIDTH
#define CFG_SYS_HELP_CMD_WIDTH	10
#endif

#ifndef	__ASSEMBLY__

/* For ARRAY_SIZE() */
#include <linux/kernel.h>

/*
 * Monitor Command Table
 */

struct cmd_tbl {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
					/*
					 * Same as ->cmd() except the command
					 * tells us if it can be repeated.
					 * Replaces the old ->repeatable field
					 * which was not able to make
					 * repeatable property different for
					 * the main command and sub-commands.
					 */
	int		(*cmd_rep)(struct cmd_tbl *cmd, int flags, int argc,
				   char *const argv[], int *repeatable);
					/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl *cmd, int flags, int argc,
			       char *const argv[]);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CONFIG_SYS_LONGHELP
	const char	*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char *const argv[],
				    char last_char, int maxv, char *cmdv[]);
#endif
};

/**
 * cmd_arg_get() - Get a particular argument
 *
 * @argc: Number of arguments
 * @argv: Argument vector of length @argc
 * @argnum: Argument to get (0=first)
 * Return: Pointer to argument @argnum if it exists, else NULL
 */
static inline const char *cmd_arg_get(int argc, char *const argv[], int argnum)
{
	return argc > argnum ? argv[argnum] : NULL;
}

static inline const char *cmd_arg0(int argc, char *const argv[])
{
	return cmd_arg_get(argc, argv, 0);
}

static inline const char *cmd_arg1(int argc, char *const argv[])
{
	return cmd_arg_get(argc, argv, 1);
}

static inline const char *cmd_arg2(int argc, char *const argv[])
{
	return cmd_arg_get(argc, argv, 2);
}

static inline const char *cmd_arg3(int argc, char *const argv[])
{
	return cmd_arg_get(argc, argv, 3);
}

#if defined(CONFIG_CMD_RUN)
int do_run(struct cmd_tbl *cmdtp, int flag, int argc,
	   char *const argv[]);
#endif

/* common/command.c */
int _do_help(struct cmd_tbl *cmd_start, int cmd_items, struct cmd_tbl *cmdtp,
	     int flag, int argc, char *const argv[]);
struct cmd_tbl *find_cmd(const char *cmd);
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     int table_len);
int complete_subcmdv(struct cmd_tbl *cmdtp, int count, int argc,
		     char *const argv[], char last_char, int maxv,
		     char *cmdv[]);

int cmd_usage(const struct cmd_tbl *cmdtp);

/* Dummy ->cmd and ->cmd_rep wrappers. */
int cmd_always_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[], int *repeatable);
int cmd_never_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[], int *repeatable);
int cmd_discard_repeatable(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[]);

static inline bool cmd_is_repeatable(struct cmd_tbl *cmdtp)
{
	return cmdtp->cmd_rep == cmd_always_repeatable;
}

#ifdef CONFIG_AUTO_COMPLETE
int var_complete(int argc, char *const argv[], char last_char, int maxv,
		 char *cmdv[]);
int cmd_auto_complete(const char *const prompt, char *buf, int *np,
		      int *colp);
#else
static inline int cmd_auto_complete(const char *const prompt, char *buf,
				    int *np, int *colp)
{
	return 0;
}
#endif

/**
 * cmd_process_error() - report and process a possible error
 *
 * @cmdtp: Command which caused the error
 * @err: Error code (0 if none, -ve for error, like -EIO)
 * Return: 0 (CMD_RET_SUCCESS) if there is not error,
 *	   1 (CMD_RET_FAILURE) if an error is found
 *	   -1 (CMD_RET_USAGE) if 'usage' error is found
 */
int cmd_process_error(struct cmd_tbl *cmdtp, int err);

/*
 * Monitor Command
 *
 * All commands use a common argument format:
 *
 * void function(struct cmd_tbl *cmdtp, int flag, int argc,
 *		 char *const argv[]);
 */

#if defined(CONFIG_CMD_MEMORY) || \
	defined(CONFIG_CMD_I2C) || \
	defined(CONFIG_CMD_ITEST) || \
	defined(CONFIG_CMD_PCI) || \
	defined(CONFIG_CMD_SETEXPR)
#define CMD_DATA_SIZE
#define CMD_DATA_SIZE_ERR	(-1)
#define CMD_DATA_SIZE_STR	(-2)

/**
 * cmd_get_data_size() - Get the data-size specifier from a command
 *
 * This reads a '.x' size specifier appended to a command. For example 'md.b'
 * is the 'md' command with a '.b' specifier, meaning that the command should
 * use bytes.
 *
 * Valid characters are:
 *
 *	b - byte
 *	w - word (16 bits)
 *	l - long (32 bits)
 *	q - quad (64 bits)
 *	s - string
 *
 * @arg: Pointers to the command to check. If a valid specifier is present it
 *	will be the last character of the string, following a '.'
 * @default_size: Default size to return if there is no specifier
 * Return: data size in bytes (1, 2, 4, 8) or CMD_DATA_SIZE_ERR for an invalid
 *	character, or CMD_DATA_SIZE_STR for a string
 */
int cmd_get_data_size(const char *arg, int default_size);
#endif

#ifdef CONFIG_CMD_BOOTD
int do_bootd(struct cmd_tbl *cmdtp, int flag, int argc,
	     char *const argv[]);
#endif
int do_bootm(struct cmd_tbl *cmdtp, int flag, int argc,
	     char *const argv[]);
#ifdef CONFIG_CMD_BOOTM
int bootm_maybe_autostart(struct cmd_tbl *cmdtp, const char *cmd);
#else
static inline int bootm_maybe_autostart(struct cmd_tbl *cmdtp, const char *cmd)
{
	return 0;
}
#endif

int do_bootz(struct cmd_tbl *cmdtp, int flag, int argc,
	     char *const argv[]);

int do_booti(struct cmd_tbl *cmdtp, int flag, int argc,
	     char *const argv[]);

int do_zboot_parent(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[], int *repeatable);

int common_diskboot(struct cmd_tbl *cmdtp, const char *intf, int argc,
		    char *const argv[]);

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc,
	     char *const argv[]);
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[]);

unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
			 char *const argv[]);

#if defined(CONFIG_CMD_NVEDIT_EFI)
int do_env_print_efi(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[]);
int do_env_set_efi(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[]);
#endif

/**
 * setexpr_regex_sub() - Replace a regex pattern with a string
 *
 * @data: Buffer containing the string to update
 * @data_size: Size of buffer (must be large enough for the new string)
 * @nbuf: Back-reference buffer
 * @nbuf_size: Size of back-reference buffer (must be larger enough for @s plus
 *	all back-reference expansions)
 * @r: Regular expression to find
 * @s: String to replace with
 * @global: true to replace all matches in @data, false to replace just the
 *	first
 * Return: 0 if OK, 1 on error
 */
int setexpr_regex_sub(char *data, uint data_size, char *nbuf, uint nbuf_size,
		      const char *r, const char *s, bool global);

/*
 * Error codes that commands return to cmd_process(). We use the standard 0
 * and 1 for success and failure, but add one more case - failure with a
 * request to call cmd_usage(). But the cmd_process() function handles
 * CMD_RET_USAGE itself and after calling cmd_usage() it will return 1.
 * This is just a convenience for commands to avoid them having to call
 * cmd_usage() all over the place.
 */
enum command_ret_t {
	CMD_RET_SUCCESS,	/* 0 = Success */
	CMD_RET_FAILURE,	/* 1 = Failure */
	CMD_RET_USAGE = -1,	/* Failure, please report 'usage' error */
};

/**
 * Process a command with arguments. We look up the command and execute it
 * if valid. Otherwise we print a usage message.
 *
 * @param flag		Some flags normally 0 (see CMD_FLAG_.. above)
 * @param argc		Number of arguments (arg 0 must be the command text)
 * @param argv		Arguments
 * @param repeatable	This function sets this to 0 if the command is not
 *			repeatable. If the command is repeatable, the value
 *			is left unchanged.
 * @param ticks		If ticks is not null, this function set it to the
 *			number of ticks the command took to complete.
 * Return: 0 if command succeeded, else non-zero (CMD_RET_...)
 */
enum command_ret_t cmd_process(int flag, int argc, char *const argv[],
			       int *repeatable, unsigned long *ticks);

void fixup_cmdtable(struct cmd_tbl *cmdtp, int size);

/**
 * board_run_command() - Fallback function to execute a command
 *
 * When no command line features are enabled in U-Boot, this function is
 * called to execute a command. Typically the function can look at the
 * command and perform a few very specific tasks, such as booting the
 * system in a particular way.
 *
 * This function is only used when CONFIG_CMDLINE is not enabled.
 *
 * In normal situations this function should not return, since U-Boot will
 * simply hang.
 *
 * @cmdline:	Command line string to execute
 * Return: 0 if OK, 1 for error
 */
int board_run_command(const char *cmdline);

int run_command(const char *cmd, int flag);
int run_command_repeatable(const char *cmd, int flag);

/**
 * run_commandf() - Run a command created by a format string
 *
 * @fmt: printf() format string
 * @...: Arguments to use (flag is always 0)
 *
 * The command cannot be larger than (CONFIG_SYS_CBSIZE - 1) characters.
 *
 * Return:
 * Returns 0 on success, -EIO if internal output error occurred, -ENOSPC in
 *	case of 'fmt' string truncation, or != 0 on error, specific for
 *	run_command().
 */
int run_commandf(const char *fmt, ...) __printf(1, 2);

/**
 * Run a list of commands separated by ; or even \0
 *
 * Note that if 'len' is not -1, then the command does not need to be nul
 * terminated, Memory will be allocated for the command in that case.
 *
 * @param cmd	List of commands to run, each separated bu semicolon
 * @param len	Length of commands excluding terminator if known (-1 if not)
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 on success, or != 0 on error.
 */
int run_command_list(const char *cmd, int len, int flag);

/**
 * cmd_source_script() - Execute a script
 *
 * Executes a U-Boot script at a particular address in memory. The script should
 * have a header (FIT or legacy) with the script type (IH_TYPE_SCRIPT).
 *
 * @addr: Address of script
 * @fit_uname: FIT subimage name
 * Return: result code (enum command_ret_t)
 */
int cmd_source_script(ulong addr, const char *fit_uname, const char *confname);
#endif	/* __ASSEMBLY__ */

/*
 * Command Flags:
 */
#define CMD_FLAG_REPEAT		0x0001	/* repeat last command		*/
#define CMD_FLAG_BOOTD		0x0002	/* command is from bootd	*/
#define CMD_FLAG_ENV		0x0004	/* command is from the environment */

#ifdef CONFIG_AUTO_COMPLETE
# define _CMD_COMPLETE(x) x,
#else
# define _CMD_COMPLETE(x)
#endif
#ifdef CONFIG_SYS_LONGHELP
# define _CMD_HELP(x) x,
#else
# define _CMD_HELP(x)
#endif

#define U_BOOT_LONGHELP(_cmdname, text)					\
	static __maybe_unused const char _cmdname##_help_text[] = text

#define U_BOOT_SUBCMDS_DO_CMD(_cmdname)					\
	static int do_##_cmdname(struct cmd_tbl *cmdtp, int flag,	\
				 int argc, char *const argv[],		\
				 int *repeatable)			\
	{								\
		struct cmd_tbl *subcmd;					\
									\
		/* We need at least the cmd and subcmd names. */	\
		if (argc < 2 || argc > CONFIG_SYS_MAXARGS)		\
			return CMD_RET_USAGE;				\
									\
		subcmd = find_cmd_tbl(argv[1], _cmdname##_subcmds,	\
				      ARRAY_SIZE(_cmdname##_subcmds));	\
		if (!subcmd || argc - 1 > subcmd->maxargs)		\
			return CMD_RET_USAGE;				\
									\
		if (flag == CMD_FLAG_REPEAT &&				\
		    !cmd_is_repeatable(subcmd))				\
			return CMD_RET_SUCCESS;				\
									\
		return subcmd->cmd_rep(subcmd, flag, argc - 1,		\
				       argv + 1, repeatable);		\
	}

#ifdef CONFIG_AUTO_COMPLETE
#define U_BOOT_SUBCMDS_COMPLETE(_cmdname)				\
	static int complete_##_cmdname(int argc, char *const argv[],	\
				       char last_char, int maxv,	\
				       char *cmdv[])			\
	{								\
		return complete_subcmdv(_cmdname##_subcmds,		\
					ARRAY_SIZE(_cmdname##_subcmds),	\
					argc - 1, argv + 1, last_char,	\
					maxv, cmdv);			\
	}
#else
#define U_BOOT_SUBCMDS_COMPLETE(_cmdname)
#endif

#define U_BOOT_SUBCMDS(_cmdname, ...)					\
	static struct cmd_tbl _cmdname##_subcmds[] = { __VA_ARGS__ };	\
	U_BOOT_SUBCMDS_DO_CMD(_cmdname)					\
	U_BOOT_SUBCMDS_COMPLETE(_cmdname)

#if CONFIG_IS_ENABLED(CMDLINE)
#define U_BOOT_CMDREP_MKENT_COMPLETE(_name, _maxargs, _cmd_rep,		\
				     _usage, _help, _comp)		\
		{ #_name, _maxargs, _cmd_rep, cmd_discard_repeatable,	\
		  _usage, _CMD_HELP(_help) _CMD_COMPLETE(_comp) }

#define U_BOOT_CMD_MKENT_COMPLETE(_name, _maxargs, _rep, _cmd,		\
				_usage, _help, _comp)			\
		{ #_name, _maxargs,					\
		 _rep ? cmd_always_repeatable : cmd_never_repeatable,	\
		 _cmd, _usage, _CMD_HELP(_help) _CMD_COMPLETE(_comp) }

#define U_BOOT_CMD_COMPLETE(_name, _maxargs, _rep, _cmd, _usage, _help, _comp) \
	ll_entry_declare(struct cmd_tbl, _name, cmd) =			\
		U_BOOT_CMD_MKENT_COMPLETE(_name, _maxargs, _rep, _cmd,	\
						_usage, _help, _comp)

#define U_BOOT_CMDREP_COMPLETE(_name, _maxargs, _cmd_rep, _usage,	\
			       _help, _comp)				\
	ll_entry_declare(struct cmd_tbl, _name, cmd) =			\
		U_BOOT_CMDREP_MKENT_COMPLETE(_name, _maxargs, _cmd_rep,	\
					     _usage, _help, _comp)

#else
#define U_BOOT_SUBCMD_START(name)	static struct cmd_tbl name[] = {};
#define U_BOOT_SUBCMD_END

#define _CMD_REMOVE(_name, _cmd)					\
	int __remove_ ## _name(void)					\
	{								\
		if (0)							\
			_cmd(NULL, 0, 0, NULL);				\
		return 0;						\
	}

#define _CMD_REMOVE_REP(_name, _cmd)					\
	int __remove_ ## _name(void)					\
	{								\
		if (0)							\
			_cmd(NULL, 0, 0, NULL, NULL);			\
		return 0;						\
	}

#define U_BOOT_CMDREP_MKENT_COMPLETE(_name, _maxargs, _cmd_rep,		\
				     _usage, _help, _comp)		\
		{ #_name, _maxargs, 0 ? _cmd_rep : NULL, NULL, _usage,	\
			_CMD_HELP(_help) _CMD_COMPLETE(_comp) }

#define U_BOOT_CMD_MKENT_COMPLETE(_name, _maxargs, _rep, _cmd, _usage,	\
				  _help, _comp)				\
		{ #_name, _maxargs, NULL, 0 ? _cmd : NULL, _usage,	\
			_CMD_HELP(_help) _CMD_COMPLETE(_comp) }

#define U_BOOT_CMD_COMPLETE(_name, _maxargs, _rep, _cmd, _usage, _help,	\
			    _comp)				\
	_CMD_REMOVE(sub_ ## _name, _cmd)

#define U_BOOT_CMDREP_COMPLETE(_name, _maxargs, _cmd_rep, _usage,	\
			       _help, _comp)				\
	_CMD_REMOVE_REP(sub_ ## _name, _cmd_rep)

#endif /* CONFIG_CMDLINE */

#define U_BOOT_CMD(_name, _maxargs, _rep, _cmd, _usage, _help)		\
	U_BOOT_CMD_COMPLETE(_name, _maxargs, _rep, _cmd, _usage, _help, NULL)

#define U_BOOT_CMD_MKENT(_name, _maxargs, _rep, _cmd, _usage, _help)	\
	U_BOOT_CMD_MKENT_COMPLETE(_name, _maxargs, _rep, _cmd,		\
					_usage, _help, NULL)

#define U_BOOT_SUBCMD_MKENT_COMPLETE(_name, _maxargs, _rep, _do_cmd,	\
				     _comp)				\
	U_BOOT_CMD_MKENT_COMPLETE(_name, _maxargs, _rep, _do_cmd,	\
				  "", "", _comp)

#define U_BOOT_SUBCMD_MKENT(_name, _maxargs, _rep, _do_cmd)		\
	U_BOOT_SUBCMD_MKENT_COMPLETE(_name, _maxargs, _rep, _do_cmd,	\
				     NULL)

#define U_BOOT_CMD_WITH_SUBCMDS(_name, _usage, _help, ...)		\
	U_BOOT_SUBCMDS(_name, __VA_ARGS__)				\
	U_BOOT_CMDREP_COMPLETE(_name, CONFIG_SYS_MAXARGS, do_##_name,	\
			       _usage, _help, complete_##_name)

#endif	/* __COMMAND_H */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Google, Inc
 * Simon Glass <sjg@chromium.org>
 */

#ifndef __CLI_H
#define __CLI_H

/**
 * Go into the command loop
 *
 * This will return if we get a timeout waiting for a command. See
 * CONFIG_BOOT_RETRY_TIME.
 */
void cli_simple_loop(void);

/**
 * cli_simple_run_command() - Execute a command with the simple CLI
 *
 * @cmd:	String containing the command to execute
 * @flag	Flag value - see CMD_FLAG_...
 * @return 1  - command executed, repeatable
 *	0  - command executed but not repeatable, interrupted commands are
 *	     always considered not repeatable
 *	-1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CONFIG_SYS_CBSIZE-1 it is
 *           considered unrecognized)
 */
int cli_simple_run_command(const char *cmd, int flag);

/**
 * cli_simple_process_macros() - Expand $() and ${} format env. variables
 *
 * @param input		Input string possible containing $() / ${} vars
 * @param output	Output string with $() / ${} vars expanded
 */
void cli_simple_process_macros(const char *input, char *output);

/**
 * cli_simple_run_command_list() - Execute a list of command
 *
 * The commands should be separated by ; or \n and will be executed
 * by the built-in parser.
 *
 * This function cannot take a const char * for the command, since if it
 * finds newlines in the string, it replaces them with \0.
 *
 * @param cmd	String containing list of commands
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int cli_simple_run_command_list(char *cmd, int flag);

/**
 * cli_readline() - read a line into the console_buffer
 *
 * This is a convenience function which calls cli_readline_into_buffer().
 *
 * @prompt: Prompt to display
 * @return command line length excluding terminator, or -ve on error
 */
int cli_readline(const char *const prompt);

/**
 * readline_into_buffer() - read a line into a buffer
 *
 * Display the prompt, then read a command line into @buffer. The
 * maximum line length is CONFIG_SYS_CBSIZE including a \0 terminator, which
 * will always be added.
 *
 * The command is echoed as it is typed. Command editing is supported if
 * CONFIG_CMDLINE_EDITING is defined. Tab auto-complete is supported if
 * CONFIG_AUTO_COMPLETE is defined. If CONFIG_BOOT_RETRY_TIME is defined,
 * then a timeout will be applied.
 *
 * If CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 *
 * @prompt:	Prompt to display
 * @buffer:	Place to put the line that is entered
 * @timeout:	Timeout in milliseconds, 0 if none
 * @return command line length excluding terminator, or -ve on error: of the
 * timeout is exceeded (either CONFIG_BOOT_RETRY_TIME or the timeout
 * parameter), then -2 is returned. If a break is detected (Ctrl-C) then
 * -1 is returned.
 */
int cli_readline_into_buffer(const char *const prompt, char *buffer,
				int timeout);

/**
 * parse_line() - split a command line down into separate arguments
 *
 * The argv[] array is filled with pointers into @line, and each argument
 * is terminated by \0 (i.e. @line is changed in the process unless there
 * is only one argument).
 *
 * #argv is terminated by a NULL after the last argument pointer.
 *
 * At most CONFIG_SYS_MAXARGS arguments are permited - if there are more
 * than that then an error is printed, and this function returns
 * CONFIG_SYS_MAXARGS, with argv[] set up to that point.
 *
 * @line:	Command line to parse
 * @args:	Array to hold arguments
 * @return number of arguments
 */
int cli_simple_parse_line(char *line, char *argv[]);

extern void (*cli_jobs_cb)(void);

#if CONFIG_IS_ENABLED(OF_CONTROL)
/**
 * cli_process_fdt() - process the boot command from the FDT
 *
 * If bootcmmd is defined in the /config node of the FDT, we use that
 * as the boot command. Further, if bootsecure is set to 1 (in the same
 * node) then we return true, indicating that the command should be executed
 * as securely as possible, avoiding the CLI parser.
 *
 * @cmdp:	On entry, the command that will be executed if the FDT does
 *		not have a command. Returns the command to execute after
 *		checking the FDT.
 * @return true to execute securely, else false
 */
bool cli_process_fdt(const char **cmdp);

/** cli_secure_boot_cmd() - execute a command as securely as possible
 *
 * This avoids using the parser, thus executing the command with the
 * smallest amount of code. Parameters are not supported.
 */
void cli_secure_boot_cmd(const char *cmd);
#else
static inline bool cli_process_fdt(const char **cmdp)
{
	return false;
}

static inline void cli_secure_boot_cmd(const char *cmd)
{
}
#endif /* CONFIG_OF_CONTROL */

/**
 * Go into the command loop
 *
 * This will return if we get a timeout waiting for a command, but only for
 * the simple parser (not hush). See CONFIG_BOOT_RETRY_TIME.
 */
void cli_loop(void);

/** Set up the command line interpreter ready for action */
void cli_init(void);

#define endtick(seconds) (get_ticks() + (uint64_t)(seconds) * get_tbclk())

#endif

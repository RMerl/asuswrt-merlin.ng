/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Google, Inc
 * Simon Glass <sjg@chromium.org>
 */

#ifndef __CLI_H
#define __CLI_H

#include <stdbool.h>
#include <linux/types.h>

/**
 * struct cli_ch_state - state information for reading cmdline characters
 *
 * @esc_len: Number of escape characters read so far
 * @esc_save: Escape characters collected so far
 * @emit_upto: Next index to emit from esc_save
 * @emitting: true if emitting from esc_save
 */
struct cli_ch_state {
	int esc_len;
	char esc_save[8];
	int emit_upto;
	bool emitting;
};

/**
 * struct cli_line_state - state of the line editor
 *
 * @num: Current cursor position, where 0 is the start
 * @eol_num: Number of characters in the buffer
 * @insert: true if in 'insert' mode
 * @history: true if history should be accessible
 * @cmd_complete: true if tab completion should be enabled (requires @prompt to
 *	be set)
 * @buf: Buffer containing line
 * @prompt: Prompt for the line
 */
struct cli_line_state {
	uint num;
	uint eol_num;
	uint len;
	bool insert;
	bool history;
	bool cmd_complete;
	char *buf;
	const char *prompt;
};

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
 * Return: 1  - command executed, repeatable
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
 * @param max_size	Maximum size of @output (including terminator)
 * Return: 0 if OK, -ENOSPC if we ran out of space in @output
 */
int cli_simple_process_macros(const char *input, char *output, int max_size);

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
 * Return: 0 on success, or != 0 on error.
 */
int cli_simple_run_command_list(char *cmd, int flag);

/**
 * cli_readline() - read a line into the console_buffer
 *
 * This is a convenience function which calls cli_readline_into_buffer().
 *
 * @prompt: Prompt to display
 * Return: command line length excluding terminator, or -ve on error
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
 * @timeout:	Timeout in seconds, 0 if none
 * Return: command line length excluding terminator, or -ve on error: if the
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
 * Return: number of arguments
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
 * Return: true to execute securely, else false
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
#define CTL_CH(c)		((c) - 'a' + 1)

/**
 * cli_ch_init() - Set up the initial state to process input characters
 *
 * @cch: State to set up
 */
void cli_ch_init(struct cli_ch_state *cch);

/**
 * cli_ch_process() - Process an input character
 *
 * When @ichar is 0, this function returns any characters from an invalid escape
 * sequence which are still pending in the buffer
 *
 * Otherwise it processes the input character. If it is an escape character,
 * then an escape sequence is started and the function returns 0. If we are in
 * the middle of an escape sequence, the character is processed and may result
 * in returning 0 (if more characters are needed) or a valid character (if
 * @ichar finishes the sequence).
 *
 * If @ichar is a valid character and there is no escape sequence in progress,
 * then it is returned as is.
 *
 * If the Enter key is pressed, '\n' is returned.
 *
 * Usage should be like this::
 *
 *    struct cli_ch_state cch;
 *
 *    cli_ch_init(cch);
 *    do
 *       {
 *       int ichar, ch;
 *
 *       ichar = cli_ch_process(cch, 0);
 *       if (!ichar) {
 *          ch = getchar();
 *          ichar = cli_ch_process(cch, ch);
 *       }
 *       (handle the ichar character)
 *    } while (!done)
 *
 * If tstc() is used to look for keypresses, this function can be called with
 * @ichar set to -ETIMEDOUT if there is no character after 5-10ms. This allows
 * the ambgiuity between the Escape key and the arrow keys (which generate an
 * escape character followed by other characters) to be resolved.
 *
 * @cch: Current state
 * @ichar: Input character to process, or 0 if none, or -ETIMEDOUT if no
 * character has been received within a small number of milliseconds (this
 * cancels any existing escape sequence and allows pressing the Escape key to
 * work)
 * Returns: Resulting input character after processing, 0 if none, '\e' if
 * an existing escape sequence was cancelled
 */
int cli_ch_process(struct cli_ch_state *cch, int ichar);

/**
 * cread_line_process_ch() - Process a character for line input
 *
 * @cls: CLI line state
 * @ichar: Character to process
 * Return: 0 if input is complete, with line in cls->buf, -EINTR if input was
 * cancelled with Ctrl-C, -EAGAIN if more characters are needed
 */
int cread_line_process_ch(struct cli_line_state *cls, char ichar);

/**
 * cli_cread_init() - Set up a new cread struct
 *
 * Sets up a new cread state, with history and cmd_complete set to false
 *
 * After calling this, you can use cread_line_process_ch() to process characters
 * received from the user.
 *
 * @cls: CLI line state
 * @buf: Text buffer containing the initial text
 * @buf_size: Buffer size, including nul terminator
 */
void cli_cread_init(struct cli_line_state *cls, char *buf, uint buf_size);

/** cread_print_hist_list() - Print the command-line history list */
void cread_print_hist_list(void);

#endif

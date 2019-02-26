/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup process process
 * @{ @ingroup utils
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <utils/utils.h>

typedef struct process_t process_t;

/**
 * Child process spawning abstraction
 */
struct process_t {

	/**
	 * Wait for a started process to terminate.
	 *
	 * The process object gets destroyed by this call, regardless of the
	 * return value.
	 *
	 * The returned code is the exit code, not the status returned by waitpid().
	 * If the program could not be executed or has terminated abnormally
	 * (by signals etc.), FALSE is returned.
	 *
	 * @param code	process exit code, set only if TRUE returned
	 * @return		TRUE if program exited normally through exit()
	 */
	bool (*wait)(process_t *this, int *code);
};

/**
 * Spawn a child process with redirected I/O.
 *
 * Forks the current process, optionally redirects stdin/out/err to the current
 * process, and executes the provided program with arguments.
 *
 * The process to execute is specified as argv[0], followed by the process
 * arguments, followed by NULL. envp[] has a NULL terminated list of arguments
 * to invoke the process with.
 *
 * If any of in/out/err is given, stdin/out/err from the child process get
 * connected over pipe()s to the caller. If close_all is TRUE, all other
 * open file descriptors get closed, regardless of any CLOEXEC setting.
 *
 * A caller must close all of the returned file descriptors to avoid file
 * descriptor leaks.
 *
 * A non-NULL return value does not guarantee that the process has been
 * invoked successfully.
 *
 * @param argv		NULL terminated process arguments, with argv[0] as program
 * @param envp		NULL terminated list of environment variables
 * @param in		pipe fd returned for redirecting data to child stdin
 * @param out		pipe fd returned to redirect child stdout data to
 * @param err		pipe fd returned to redirect child stderr data to
 * @param close_all	close all open file descriptors above 2 before execve()
 * @return			process, NULL on failure
 */
process_t* process_start(char *const argv[], char *const envp[],
						 int *in, int *out, int *err, bool close_all);

/**
 * Spawn a command in a shell child process.
 *
 * Same as process_start(), but passes a single command to a shell, such as
 * "sh -c". See process_start() for I/O redirection notes.
 *
 * @param envp		NULL terminated list of environment variables
 * @param in		pipe fd returned for redirecting data to child stdin
 * @param out		pipe fd returned to redirect child stdout data to
 * @param err		pipe fd returned to redirect child stderr data to
 * @param fmt		printf format string for command
 * @param ...		arguments for fmt
 * @return			process, NULL on failure
 */
process_t* process_start_shell(char *const envp[], int *in, int *out, int *err,
							   char *fmt, ...);

#endif /** PROCESS_H_ @}*/

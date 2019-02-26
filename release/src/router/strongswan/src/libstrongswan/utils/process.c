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

/* vasprintf() */
#define _GNU_SOURCE
#include "process.h"

#include <library.h>
#include <utils/debug.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct private_process_t private_process_t;

/**
 * Ends of a pipe()
 */
enum {
	PIPE_READ = 0,
	PIPE_WRITE = 1,
	PIPE_ENDS,
};

#ifndef WIN32

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

/**
 * Private data of an process_t object.
 */
struct private_process_t {

	/**
	 * Public process_t interface.
	 */
	process_t public;

	/**
	 * child stdin pipe
	 */
	int in[PIPE_ENDS];

	/**
	 * child stdout pipe
	 */
	int out[PIPE_ENDS];

	/**
	 * child stderr pipe
	 */
	int err[PIPE_ENDS];

	/**
	 * child process
	 */
	int pid;
};

/**
 * Close a file descriptor if it is not -1
 */
static void close_if(int *fd)
{
	if (*fd != -1)
	{
		close(*fd);
		*fd = -1;
	}
}

/**
 * Destroy a process structure, close all pipes
 */
static void process_destroy(private_process_t *this)
{
	close_if(&this->in[PIPE_READ]);
	close_if(&this->in[PIPE_WRITE]);
	close_if(&this->out[PIPE_READ]);
	close_if(&this->out[PIPE_WRITE]);
	close_if(&this->err[PIPE_READ]);
	close_if(&this->err[PIPE_WRITE]);
	free(this);
}

METHOD(process_t, wait_, bool,
	private_process_t *this, int *code)
{
	int status, ret;

	ret = waitpid(this->pid, &status, 0);
	process_destroy(this);
	if (ret == -1)
	{
		return FALSE;
	}
	if (!WIFEXITED(status))
	{
		return FALSE;
	}
	if (code)
	{
		*code = WEXITSTATUS(status);
	}
	return TRUE;
}

/**
 * See header
 */
process_t* process_start(char *const argv[], char *const envp[],
						 int *in, int *out, int *err, bool close_all)
{
	private_process_t *this;
	char *empty[] = { NULL };

	INIT(this,
		.public = {
			.wait = _wait_,
		},
		.in = { -1, -1 },
		.out = { -1, -1 },
		.err = { -1, -1 },
	);

	if (in && pipe(this->in) != 0)
	{
		DBG1(DBG_LIB, "creating stdin pipe failed: %s", strerror(errno));
		process_destroy(this);
		return NULL;
	}
	if (out && pipe(this->out) != 0)
	{
		DBG1(DBG_LIB, "creating stdout pipe failed: %s", strerror(errno));
		process_destroy(this);
		return NULL;
	}
	if (err && pipe(this->err) != 0)
	{
		DBG1(DBG_LIB, "creating stderr pipe failed: %s", strerror(errno));
		process_destroy(this);
		return NULL;
	}

	this->pid = fork();
	switch (this->pid)
	{
		case -1:
			DBG1(DBG_LIB, "forking process failed: %s", strerror(errno));
			process_destroy(this);
			return NULL;
		case 0:
			/* child */
			close_if(&this->in[PIPE_WRITE]);
			close_if(&this->out[PIPE_READ]);
			close_if(&this->err[PIPE_READ]);
			if (this->in[PIPE_READ] != -1)
			{
				if (dup2(this->in[PIPE_READ], 0) == -1)
				{
					raise(SIGKILL);
				}
			}
			if (this->out[PIPE_WRITE] != -1)
			{
				if (dup2(this->out[PIPE_WRITE], 1) == -1)
				{
					raise(SIGKILL);
				}
			}
			if (this->err[PIPE_WRITE] != -1)
			{
				if (dup2(this->err[PIPE_WRITE], 2) == -1)
				{
					raise(SIGKILL);
				}
			}
			if (close_all)
			{
				closefrom(3);
			}
			if (execve(argv[0], argv, envp ?: empty) == -1)
			{
				raise(SIGKILL);
			}
			/* not reached */
		default:
			/* parent */
			close_if(&this->in[PIPE_READ]);
			close_if(&this->out[PIPE_WRITE]);
			close_if(&this->err[PIPE_WRITE]);
			if (in)
			{
				*in = this->in[PIPE_WRITE];
				this->in[PIPE_WRITE] = -1;
			}
			if (out)
			{
				*out = this->out[PIPE_READ];
				this->out[PIPE_READ] = -1;
			}
			if (err)
			{
				*err = this->err[PIPE_READ];
				this->err[PIPE_READ] = -1;
			}
			return &this->public;
	}
}

/**
 * See header
 */
process_t* process_start_shell(char *const envp[], int *in, int *out, int *err,
							   char *fmt, ...)
{
	char *argv[] = {
		"/bin/sh",
		"-c",
		NULL,
		NULL
	};
	process_t *process;
	va_list args;
	int len;

	va_start(args, fmt);
	len = vasprintf(&argv[2], fmt, args);
	va_end(args);
	if (len < 0)
	{
		return NULL;
	}

	process = process_start(argv, envp, in, out, err, TRUE);
	free(argv[2]);
	return process;
}

#else /* WIN32 */

/**
 * Private data of an process_t object.
 */
struct private_process_t {

	/**
	 * Public process_t interface.
	 */
	process_t public;

	/**
	 * child stdin pipe
	 */
	HANDLE in[PIPE_ENDS];

	/**
	 * child stdout pipe
	 */
	HANDLE out[PIPE_ENDS];

	/**
	 * child stderr pipe
	 */
	HANDLE err[PIPE_ENDS];

	/**
	 * child process information
	 */
	PROCESS_INFORMATION pi;
};

/**
 * Clean up state associated to child process
 */
static void process_destroy(private_process_t *this)
{
	if (this->in[PIPE_READ])
	{
		CloseHandle(this->in[PIPE_READ]);
	}
	if (this->in[PIPE_WRITE])
	{
		CloseHandle(this->in[PIPE_WRITE]);
	}
	if (this->out[PIPE_READ])
	{
		CloseHandle(this->out[PIPE_READ]);
	}
	if (this->out[PIPE_WRITE])
	{
		CloseHandle(this->out[PIPE_WRITE]);
	}
	if (this->err[PIPE_READ])
	{
		CloseHandle(this->err[PIPE_READ]);
	}
	if (this->err[PIPE_WRITE])
	{
		CloseHandle(this->err[PIPE_WRITE]);
	}
	if (this->pi.hProcess)
	{
		CloseHandle(this->pi.hProcess);
		CloseHandle(this->pi.hThread);
	}
	free(this);
}

METHOD(process_t, wait_, bool,
	private_process_t *this, int *code)
{
	DWORD ec;

	if (WaitForSingleObject(this->pi.hProcess, INFINITE) != WAIT_OBJECT_0)
	{
		DBG1(DBG_LIB, "waiting for child process failed: 0x%08x",
			 GetLastError());
		process_destroy(this);
		return FALSE;
	}
	if (code)
	{
		if (!GetExitCodeProcess(this->pi.hProcess, &ec))
		{
			DBG1(DBG_LIB, "getting child process exit code failed: 0x%08x",
				 GetLastError());
			process_destroy(this);
			return FALSE;
		}
		*code = ec;
	}
	process_destroy(this);
	return TRUE;
}

/**
 * Append a command line argument to buf, optionally quoted
 */
static void append_arg(char *buf, u_int len, char *arg, char *quote)
{
	char *space = "";
	int current;

	current = strlen(buf);
	if (current)
	{
		space = " ";
	}
	snprintf(buf + current, len - current, "%s%s%s%s", space, quote, arg, quote);
}

/**
 * Append a null-terminate env string to buf
 */
static void append_env(char *buf, u_int len, char *env)
{
	char *pos = buf;
	int current;

	while (TRUE)
	{
		pos += strlen(pos);
		if (!pos[1])
		{
			if (pos == buf)
			{
				current = 0;
			}
			else
			{
				current = pos - buf + 1;
			}
			snprintf(buf + current, len - current, "%s", env);
			break;
		}
		pos++;
	}
}

/**
 * See header
 */
process_t* process_start(char *const argv[], char *const envp[],
						 int *in, int *out, int *err, bool close_all)
{
	private_process_t *this;
	char arg[32768], env[32768];
	SECURITY_ATTRIBUTES sa = {
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = TRUE,
	};
	STARTUPINFO sui = {
		.cb = sizeof(STARTUPINFO),
	};
	int i;

	memset(arg, 0, sizeof(arg));
	memset(env, 0, sizeof(env));

	for (i = 0; argv[i]; i++)
	{
		if (!strchr(argv[i], ' '))
		{	/* no spaces, fine for appending */
			append_arg(arg, sizeof(arg) - 1, argv[i], "");
		}
		else if (argv[i][0] == '"' &&
				 argv[i][strlen(argv[i]) - 1] == '"' &&
				 strchr(argv[i] + 1, '"') == argv[i] + strlen(argv[i]) - 1)
		{	/* already properly quoted */
			append_arg(arg, sizeof(arg) - 1, argv[i], "");
		}
		else if (strchr(argv[i], ' ') && !strchr(argv[i], '"'))
		{	/* spaces, but no quotes; append quoted */
			append_arg(arg, sizeof(arg) - 1, argv[i], "\"");
		}
		else
		{
			DBG1(DBG_LIB, "invalid command line argument: %s", argv[i]);
			return NULL;
		}
	}
	if (envp)
	{
		for (i = 0; envp[i]; i++)
		{
			append_env(env, sizeof(env) - 1, envp[i]);
		}
	}

	INIT(this,
		.public = {
			.wait = _wait_,
		},
	);

	if (in)
	{
		sui.dwFlags = STARTF_USESTDHANDLES;
		if (!CreatePipe(&this->in[PIPE_READ], &this->in[PIPE_WRITE], &sa, 0))
		{
			process_destroy(this);
			return NULL;
		}
		if (!SetHandleInformation(this->in[PIPE_WRITE], HANDLE_FLAG_INHERIT, 0))
		{
			process_destroy(this);
			return NULL;
		}
		sui.hStdInput = this->in[PIPE_READ];
		*in = _open_osfhandle((uintptr_t)this->in[PIPE_WRITE], 0);
		if (*in == -1)
		{
			process_destroy(this);
			return NULL;
		}
	}
	if (out)
	{
		sui.dwFlags = STARTF_USESTDHANDLES;
		if (!CreatePipe(&this->out[PIPE_READ], &this->out[PIPE_WRITE], &sa, 0))
		{
			process_destroy(this);
			return NULL;
		}
		if (!SetHandleInformation(this->out[PIPE_READ], HANDLE_FLAG_INHERIT, 0))
		{
			process_destroy(this);
			return NULL;
		}
		sui.hStdOutput = this->out[PIPE_WRITE];
		*out = _open_osfhandle((uintptr_t)this->out[PIPE_READ], 0);
		if (*out == -1)
		{
			process_destroy(this);
			return NULL;
		}
	}
	if (err)
	{
		sui.dwFlags = STARTF_USESTDHANDLES;
		if (!CreatePipe(&this->err[PIPE_READ], &this->err[PIPE_WRITE], &sa, 0))
		{
			process_destroy(this);
			return NULL;
		}
		if (!SetHandleInformation(this->err[PIPE_READ], HANDLE_FLAG_INHERIT, 0))
		{
			process_destroy(this);
			return NULL;
		}
		sui.hStdError = this->err[PIPE_WRITE];
		*err = _open_osfhandle((uintptr_t)this->err[PIPE_READ], 0);
		if (*err == -1)
		{
			process_destroy(this);
			return NULL;
		}
	}

	if (!CreateProcess(argv[0], arg, NULL, NULL, TRUE,
					   NORMAL_PRIORITY_CLASS, env, NULL, &sui, &this->pi))
	{
		DBG1(DBG_LIB, "creating process '%s' failed: 0x%08x",
			 argv[0], GetLastError());
		process_destroy(this);
		return NULL;
	}

	/* close child process end of pipes */
	if (this->in[PIPE_READ])
	{
		CloseHandle(this->in[PIPE_READ]);
		this->in[PIPE_READ] = NULL;
	}
	if (this->out[PIPE_WRITE])
	{
		CloseHandle(this->out[PIPE_WRITE]);
		this->out[PIPE_WRITE] = NULL;
	}
	if (this->err[PIPE_WRITE])
	{
		CloseHandle(this->err[PIPE_WRITE]);
		this->err[PIPE_WRITE] = NULL;
	}
	/* our side gets closed over the osf_handle closed by caller */
	this->in[PIPE_WRITE] = NULL;
	this->out[PIPE_READ] = NULL;
	this->err[PIPE_READ] = NULL;
	return &this->public;
}

/**
 * See header
 */
process_t* process_start_shell(char *const envp[], int *in, int *out, int *err,
							   char *fmt, ...)
{
	char path[MAX_PATH], *exe = "system32\\cmd.exe";
	char *argv[] = {
		path,
		"/C",
		NULL,
		NULL
	};
	process_t *process;
	va_list args;
	int len;

	len = GetSystemWindowsDirectory(path, sizeof(path));
	if (len == 0 || len >= sizeof(path) - strlen(exe))
	{
		DBG1(DBG_LIB, "resolving Windows directory failed: 0x%08x",
			 GetLastError());
		return NULL;
	}
	if (path[len + 1] != '\\')
	{
		strncat(path, "\\", sizeof(path) - len++);
	}
	strncat(path, exe, sizeof(path) - len);

	va_start(args, fmt);
	len = vasprintf(&argv[2], fmt, args);
	va_end(args);
	if (len < 0)
	{
		return NULL;
	}

	process = process_start(argv, envp, in, out, err, TRUE);
	free(argv[2]);
	return process;
}

#endif /* WIN32 */

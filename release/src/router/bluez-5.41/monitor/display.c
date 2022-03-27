/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "display.h"

static pid_t pager_pid = 0;

bool use_color(void)
{
	static int cached_use_color = -1;

	if (__builtin_expect(!!(cached_use_color < 0), 0))
		cached_use_color = isatty(STDOUT_FILENO) > 0 || pager_pid > 0;

	return cached_use_color;
}

int num_columns(void)
{
	static int cached_num_columns = -1;

	if (__builtin_expect(!!(cached_num_columns < 0), 0)) {
		struct winsize ws;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0 ||
								ws.ws_col == 0)
			cached_num_columns = FALLBACK_TERMINAL_WIDTH;
		else
			cached_num_columns = ws.ws_col;
	}

	return cached_num_columns;
}

static void close_pipe(int p[])
{
	if (p[0] >= 0)
		close(p[0]);
	if (p[1] >= 0)
		close(p[1]);
}

static void wait_for_terminate(pid_t pid)
{
	siginfo_t dummy;

	for (;;) {
		memset(&dummy, 0, sizeof(dummy));

		if (waitid(P_PID, pid, &dummy, WEXITED) < 0) {
			if (errno == EINTR)
				continue;
			return;
		}

		return;
	}
}

void open_pager(void)
{
	const char *pager;
	pid_t parent_pid;
	int fd[2];

	if (pager_pid > 0)
		return;

	pager = getenv("PAGER");
	if (pager) {
		if (!*pager || strcmp(pager, "cat") == 0)
			return;
	}

	if (!(isatty(STDOUT_FILENO) > 0))
		return;

	num_columns();

	if (pipe(fd) < 0) {
		perror("Failed to create pager pipe");
		return;
	}

	parent_pid = getpid();

	pager_pid = fork();
	if (pager_pid < 0) {
		perror("Failed to fork pager");
		close_pipe(fd);
		return;
	}

	if (pager_pid == 0) {
		dup2(fd[0], STDIN_FILENO);
		close_pipe(fd);

		setenv("LESS", "FRSX", 0);

		if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
			_exit(EXIT_FAILURE);

		if (getppid() != parent_pid)
			_exit(EXIT_SUCCESS);

		if (pager) {
			execlp(pager, pager, NULL);
			execl("/bin/sh", "sh", "-c", pager, NULL);
		}

		execlp("pager", "pager", NULL);
		execlp("less", "less", NULL);
		execlp("more", "more", NULL);

		_exit(EXIT_FAILURE);
	}

	if (dup2(fd[1], STDOUT_FILENO) < 0) {
		perror("Failed to duplicate pager pipe");
		return;
	}

	close_pipe(fd);
}

void close_pager(void)
{
	if (pager_pid <= 0)
		return;

	fclose(stdout);
	kill(pager_pid, SIGCONT);
	wait_for_terminate(pager_pid);
	pager_pid = 0;
}

/* strongSwan charon launcher
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
 * Copyright (C) 2006 Martin Willi - Hochschule fuer Technik Rapperswil
 *
 * Ported from invokepluto.c to fit charons needs.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <library.h>
#include <utils/debug.h>

#include "confread.h"
#include "invokecharon.h"
#include "files.h"

static int _charon_pid = 0;
static int _stop_requested;

pid_t starter_charon_pid(void)
{
	return _charon_pid;
}

void starter_charon_sigchild(pid_t pid, int status)
{
	if (pid == _charon_pid)
	{
		_charon_pid = 0;
		if (status == SS_RC_LIBSTRONGSWAN_INTEGRITY ||
			status == SS_RC_DAEMON_INTEGRITY)
		{
			DBG1(DBG_APP, "%s has quit: integrity test of %s failed",
				 daemon_name, (status == 64) ? "libstrongswan" : daemon_name);
			_stop_requested = 1;
		}
		else if (status == SS_RC_INITIALIZATION_FAILED)
		{
			DBG1(DBG_APP, "%s has quit: initialization failed", daemon_name);
			_stop_requested = 1;
		}
		if (!_stop_requested)
		{
			DBG1(DBG_APP, "%s has died -- restart scheduled (%dsec)",
				 daemon_name, CHARON_RESTART_DELAY);
			alarm(CHARON_RESTART_DELAY);   // restart in 5 sec
		}
		unlink(pid_file);
	}
}

int starter_stop_charon (void)
{
	int i;
	pid_t pid = _charon_pid;

	if (pid)
	{
		_stop_requested = 1;

		/* be more and more aggressive */
		for (i = 0; i < 50 && (pid = _charon_pid) != 0; i++)
		{
			if (i == 0)
			{
				kill(pid, SIGINT);
			}
			else if (i < 40)
			{
				kill(pid, SIGTERM);
			}
			else if (i == 40)
			{
				kill(pid, SIGKILL);
				DBG1(DBG_APP, "starter_stop_charon(): %s does not respond, sending KILL",
					 daemon_name);
			}
			else
			{
				kill(pid, SIGKILL);
			}
			usleep(200000); /* sleep for 200 ms */
		}
		if (_charon_pid == 0)
		{
			DBG1(DBG_APP, "%s stopped after %d ms", daemon_name, 200*i);
			return 0;
		}
		DBG1(DBG_APP, "starter_stop_charon(): can't stop %s !!!", daemon_name);
		return -1;
	}
	else
	{
		DBG1(DBG_APP, "stater_stop_charon(): %s was not started...", daemon_name);
	}
	return -1;
}


int starter_start_charon (starter_config_t *cfg, bool no_fork, bool attach_gdb)
{
	struct stat stb;
	int pid, i;
	char buffer[BUF_LEN];
	int argc = 1;
	char *arg[] = {
		cmd, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	if (attach_gdb)
	{
		argc = 0;
		arg[argc++] = "/usr/bin/gdb";
		arg[argc++] = "--args";
		arg[argc++] = cmd;
		}
	if (!no_fork)
	{
		arg[argc++] = "--use-syslog";
	}

	/* parse debug string */
	{
		int level;
		char type[4];
		char *pos = cfg->setup.charondebug;
		char *buf_pos = buffer;

		while (pos && sscanf(pos, "%3s %d,", type, &level) == 2)
		{
			snprintf(buf_pos, buffer + sizeof(buffer) - buf_pos, "--debug-%s", type);
			arg[argc++] = buf_pos;
			buf_pos += strlen(buf_pos) + 1;
			if (buf_pos >= buffer + sizeof(buffer))
			{
				break;
			}
			snprintf(buf_pos, buffer + sizeof(buffer) - buf_pos, "%d", level);
			arg[argc++] = buf_pos;
			buf_pos += strlen(buf_pos) + 1;
			if (buf_pos >= buffer + sizeof(buffer))
			{
				break;
			}

			/* get next */
			pos = strchr(pos, ',');
			if (pos)
			{
				pos++;
			}
		}
	}

	if (_charon_pid)
	{
		DBG1(DBG_APP, "starter_start_charon(): %s already started...",
			 daemon_name);
		return -1;
	}
	else
	{
		unlink(CHARON_CTL_FILE);
		_stop_requested = 0;

		pid = fork();
		switch (pid)
		{
		case -1:
			DBG1(DBG_APP, "can't fork(): %s", strerror(errno));
			return -1;
		case 0:
			/* child */
			setsid();
			closefrom(3);
			sigprocmask(SIG_SETMASK, 0, NULL);
			/* disable glibc's malloc checker, conflicts with leak detective */
			setenv("MALLOC_CHECK_", "0", 1);
			execv(arg[0], arg);
			DBG1(DBG_APP, "can't execv(%s,...): %s", arg[0], strerror(errno));
			exit(1);
		default:
			/* father */
			_charon_pid = pid;
			while (attach_gdb)
			{
				/* wait indefinitely if gdb is attached */
				usleep(10000);
				if (stat(pid_file, &stb) == 0)
				{
					return 0;
				}
			}
			for (i = 0; i < 500 && _charon_pid; i++)
			{
				/* wait for charon for a maximum of 500 x 20 ms = 10 s */
				usleep(20000);
				if (stat(pid_file, &stb) == 0)
				{
					DBG1(DBG_APP, "%s (%d) started after %d ms", daemon_name,
						 _charon_pid, 20*(i+1));
					return 0;
				}
			}
			if (_charon_pid)
			{
				/* If charon is started but with no ctl file, stop it */
				DBG1(DBG_APP, "%s too long to start... - kill kill",
					 daemon_name);
				for (i = 0; i < 20 && (pid = _charon_pid) != 0; i++)
				{
					if (i == 0)
					{
						kill(pid, SIGINT);
					}
					else if (i < 10)
					{
						kill(pid, SIGTERM);
					}
					else
					{
						kill(pid, SIGKILL);
					}
					usleep(20000); /* sleep for 20 ms */
				}
			}
			else
			{
				DBG1(DBG_APP, "%s refused to be started", daemon_name);
			}
			return -1;
		}
	}
	return -1;
}

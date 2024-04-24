/*
 * Copyright (C) 2008-2015 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "utils.h"

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#ifndef WIN32
# include <signal.h>
#endif

#ifndef HAVE_CLOSEFROM
#if defined(__linux__) && defined(HAVE_SYS_SYSCALL_H)
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/syscall.h>
/* This is from the kernel sources.  We limit the length of directory names to
 * 256 as we only use it to enumerate FDs. */
struct linux_dirent64 {
	uint64_t d_ino;
	int64_t d_off;
	unsigned short	d_reclen;
	unsigned char d_type;
	char d_name[256];
};
#else /* !defined(__linux__) || !defined(HAVE_SYS_SYSCALL_H) */
# include <dirent.h>
#endif /* defined(__linux__) && defined(HAVE_SYS_SYSCALL_H) */
#endif

#include <library.h>
#include <collections/enumerator.h>

#define FD_DIR "/proc/self/fd"

#ifdef WIN32

#include <threading/mutex.h>
#include <threading/condvar.h>

/**
 * Flag to indicate signaled wait_sigint()
 */
static bool sigint_signaled = FALSE;

/**
 * Condvar to wait in wait_sigint()
 */
static condvar_t *sigint_cond;

/**
 * Mutex to check signaling()
 */
static mutex_t *sigint_mutex;

/**
 * Control handler to catch ^C
 */
static BOOL WINAPI handler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
			sigint_mutex->lock(sigint_mutex);
			sigint_signaled = TRUE;
			sigint_cond->signal(sigint_cond);
			sigint_mutex->unlock(sigint_mutex);
			return TRUE;
		default:
			return FALSE;
	}
}

/**
 * Windows variant
 */
void wait_sigint()
{
	SetConsoleCtrlHandler(handler, TRUE);

	sigint_mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	sigint_cond = condvar_create(CONDVAR_TYPE_DEFAULT);

	sigint_mutex->lock(sigint_mutex);
	while (!sigint_signaled)
	{
		sigint_cond->wait(sigint_cond, sigint_mutex);
	}
	sigint_mutex->unlock(sigint_mutex);

	sigint_mutex->destroy(sigint_mutex);
	sigint_cond->destroy(sigint_cond);
}

/**
 * Windows variant
 */
void send_sigint()
{
	handler(CTRL_C_EVENT);
}

#else /* !WIN32 */

/**
 * Unix variant
 */
void wait_sigint()
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);

	sigprocmask(SIG_BLOCK, &set, NULL);
	while (sigwaitinfo(&set, NULL) == -1 && errno == EINTR)
	{
		/* wait for signal */
	}
}

/**
 * Unix variant
 */
void send_sigint()
{
	kill(0, SIGINT);
}

#ifndef HAVE_SIGWAITINFO
int sigwaitinfo(const sigset_t *set, void *info)
{
	int sig, err;

	if (info)
	{	/* we don't replicate siginfo_t, fail if anybody tries to use it */
		errno = EINVAL;
		return -1;
	}
	err = sigwait(set, &sig);
	if (err != 0)
	{
		errno = err;
		sig = -1;
	}
	return sig;
}
#endif /* HAVE_SIGWAITINFO */
#endif /* WIN32 */

#ifndef HAVE_CLOSEFROM
/**
 * Described in header.
 */
void closefrom(int low_fd)
{
	int max_fd, dir_fd, fd;

	/* try to close only open file descriptors on Linux... */
#if defined(__linux__) && defined(HAVE_SYS_SYSCALL_H)
	/* By directly using a syscall we avoid any calls that might be unsafe after
	 * fork() (e.g. malloc()). */
	char buffer[sizeof(struct linux_dirent64)];
	struct linux_dirent64 *entry;
	int offset, len;

	dir_fd = open("/proc/self/fd", O_RDONLY);
	if (dir_fd != -1)
	{
		while ((len = syscall(__NR_getdents64, dir_fd, buffer,
							  sizeof(buffer))) > 0)
		{
			for (offset = 0; offset < len; offset += entry->d_reclen)
			{
				entry = (struct linux_dirent64*)(buffer + offset);
				if (!isdigit(entry->d_name[0]))
				{
					continue;
				}
				fd = atoi(entry->d_name);
				if (fd != dir_fd && fd >= low_fd)
				{
					close(fd);
				}
			}
		}
		close(dir_fd);
		return;
	}
#else /* !defined(__linux__) || !defined(HAVE_SYS_SYSCALL_H) */
	/* This is potentially unsafe when called after fork() in multi-threaded
	 * applications.  In particular opendir() will require an allocation.
	 * Depends on how the malloc() implementation handles such situations. */
	DIR *dir;
	struct dirent *entry;

#ifndef HAVE_DIRFD
	/* if we don't have dirfd() lets close the lowest FD and hope it gets reused
	 * by opendir() */
	close(low_fd);
	dir_fd = low_fd++;
#endif

	dir = opendir(FD_DIR);
	if (dir)
	{
#ifdef HAVE_DIRFD
		dir_fd = dirfd(dir);
#endif
		while ((entry = readdir(dir)))
		{
			if (!isdigit(entry->d_name[0]))
			{
				continue;
			}
			fd = atoi(entry->d_name);
			if (fd != dir_fd && fd >= low_fd)
			{
				close(fd);
			}
		}
		closedir(dir);
		return;
	}
#endif /* defined(__linux__) && defined(HAVE_SYS_SYSCALL_H) */

	/* ...fall back to closing all fds otherwise */
#ifdef WIN32
	max_fd = _getmaxstdio();
#else
	max_fd = (int)sysconf(_SC_OPEN_MAX);
#endif
	if (max_fd < 0)
	{
		max_fd = 256;
	}
	for (fd = low_fd; fd < max_fd; fd++)
	{
		close(fd);
	}
}
#endif /* HAVE_CLOSEFROM */

/**
 * return null
 */
void *return_null()
{
	return NULL;
}

/**
 * returns TRUE
 */
bool return_true()
{
	return TRUE;
}

/**
 * returns FALSE
 */
bool return_false()
{
	return FALSE;
}

/**
 * nop operation
 */
void nop()
{
}

/**
 * See header
 */
void utils_init()
{
#ifdef WIN32
	windows_init();
#endif
	atomics_init();
	strerror_init();
}

/**
 * See header
 */
void utils_deinit()
{
#ifdef WIN32
	windows_deinit();
#endif
	atomics_deinit();
	strerror_deinit();
}

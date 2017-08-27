/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#define _GNU_SOURCE /* for memrchr */
#ifdef WIN32
/* for GetTickCount64, Windows 7 */
# define _WIN32_WINNT 0x0601
#endif

#include "utils.h"

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>
#ifndef WIN32
# include <signal.h>
#endif

#include <library.h>
#include <utils/debug.h>
#include <utils/chunk.h>
#include <collections/enumerator.h>
#include <threading/spinlock.h>
#include <threading/mutex.h>
#include <threading/condvar.h>

ENUM(status_names, SUCCESS, NEED_MORE,
	"SUCCESS",
	"FAILED",
	"OUT_OF_RES",
	"ALREADY_DONE",
	"NOT_SUPPORTED",
	"INVALID_ARG",
	"NOT_FOUND",
	"PARSE_ERROR",
	"VERIFY_ERROR",
	"INVALID_STATE",
	"DESTROY_ME",
	"NEED_MORE",
);

/**
 * Described in header.
 */
void memxor(u_int8_t dst[], u_int8_t src[], size_t n)
{
	int m, i;

	/* byte wise XOR until dst aligned */
	for (i = 0; (uintptr_t)&dst[i] % sizeof(long) && i < n; i++)
	{
		dst[i] ^= src[i];
	}
	/* try to use words if src shares an aligment with dst */
	switch (((uintptr_t)&src[i] % sizeof(long)))
	{
		case 0:
			for (m = n - sizeof(long); i <= m; i += sizeof(long))
			{
				*(long*)&dst[i] ^= *(long*)&src[i];
			}
			break;
		case sizeof(int):
			for (m = n - sizeof(int); i <= m; i += sizeof(int))
			{
				*(int*)&dst[i] ^= *(int*)&src[i];
			}
			break;
		case sizeof(short):
			for (m = n - sizeof(short); i <= m; i += sizeof(short))
			{
				*(short*)&dst[i] ^= *(short*)&src[i];
			}
			break;
		default:
			break;
	}
	/* byte wise XOR of the rest */
	for (; i < n; i++)
	{
		dst[i] ^= src[i];
	}
}

/**
 * Described in header.
 */
void memwipe_noinline(void *ptr, size_t n)
{
	memwipe_inline(ptr, n);
}

/**
 * Described in header.
 */
void *memstr(const void *haystack, const char *needle, size_t n)
{
	const u_char *pos = haystack;
	size_t l;

	if (!haystack || !needle || (l = strlen(needle)) == 0)
	{
		return NULL;
	}
	for (; n >= l; ++pos, --n)
	{
		if (memeq(pos, needle, l))
		{
			return (void*)pos;
		}
	}
	return NULL;
}

/**
 * Described in header.
 */
void *utils_memrchr(const void *s, int c, size_t n)
{
	const u_char *pos;

	if (!s || !n)
	{
		return NULL;
	}

	for (pos = s + n - 1; pos >= (u_char*)s; pos--)
	{
		if (*pos == (u_char)c)
		{
			return (void*)pos;
		}
	}
	return NULL;
}

/**
 * Described in header.
 */
char* translate(char *str, const char *from, const char *to)
{
	char *pos = str;
	if (strlen(from) != strlen(to))
	{
		return str;
	}
	while (pos && *pos)
	{
		char *match;
		if ((match = strchr(from, *pos)) != NULL)
		{
			*pos = to[match - from];
		}
		pos++;
	}
	return str;
}

/**
 * Described in header.
 */
char* strreplace(const char *str, const char *search, const char *replace)
{
	size_t len, slen, rlen, count = 0;
	char *res, *pos, *found, *dst;

	if (!str || !*str || !search || !*search || !replace)
	{
		return (char*)str;
	}
	slen = strlen(search);
	rlen = strlen(replace);
	if (slen != rlen)
	{
		for (pos = (char*)str; (pos = strstr(pos, search)); pos += slen)
		{
			found = pos;
			count++;
		}
		if (!count)
		{
			return (char*)str;
		}
		len = (found - str) + strlen(found) + count * (rlen - slen);
	}
	else
	{
		len = strlen(str);
	}
	found = strstr(str, search);
	if (!found)
	{
		return (char*)str;
	}
	dst = res = malloc(len + 1);
	pos = (char*)str;
	do
	{
		len = found - pos;
		memcpy(dst, pos, len);
		dst += len;
		memcpy(dst, replace, rlen);
		dst += rlen;
		pos = found + slen;
	}
	while ((found = strstr(pos, search)));
	strcpy(dst, pos);
	return res;
}

#ifdef WIN32

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

#else /* !WIN32 */

/**
 * Unix variant
 */
void wait_sigint()
{
	sigset_t set;
	int sig;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);

	sigprocmask(SIG_BLOCK, &set, NULL);
	sigwait(&set, &sig);
}

#endif

/**
 * Described in header.
 */
char* path_dirname(const char *path)
{
	char *pos;

	pos = path ? strrchr(path, DIRECTORY_SEPARATOR[0]) : NULL;

	if (pos && !pos[1])
	{	/* if path ends with slashes we have to look beyond them */
		while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* skip trailing slashes */
			pos--;
		}
		pos = memrchr(path, DIRECTORY_SEPARATOR[0], pos - path + 1);
	}
	if (!pos)
	{
#ifdef WIN32
		if (path && strlen(path))
		{
			if ((isalpha(path[0]) && path[1] == ':'))
			{	/* if just a drive letter given, return that as dirname */
				return chunk_clone(chunk_from_chars(path[0], ':', 0)).ptr;
			}
		}
#endif
		return strdup(".");
	}
	while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
	{	/* skip superfluous slashes */
		pos--;
	}
	return strndup(path, pos - path + 1);
}

/**
 * Described in header.
 */
char* path_basename(const char *path)
{
	char *pos, *trail = NULL;

	if (!path || !*path)
	{
		return strdup(".");
	}
	pos = strrchr(path, DIRECTORY_SEPARATOR[0]);
	if (pos && !pos[1])
	{	/* if path ends with slashes we have to look beyond them */
		while (pos > path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* skip trailing slashes */
			pos--;
		}
		if (pos == path && *pos == DIRECTORY_SEPARATOR[0])
		{	/* contains only slashes */
			return strdup(DIRECTORY_SEPARATOR);
		}
		trail = pos + 1;
		pos = memrchr(path, DIRECTORY_SEPARATOR[0], trail - path);
	}
	pos = pos ? pos + 1 : (char*)path;
	return trail ? strndup(pos, trail - pos) : strdup(pos);
}

/**
 * Described in header.
 */
bool path_absolute(const char *path)
{
	if (!path)
	{
		return FALSE;
	}
#ifdef WIN32
	if (strpfx(path, "\\\\"))
	{	/* UNC */
		return TRUE;
	}
	if (strlen(path) && isalpha(path[0]) && path[1] == ':')
	{	/* drive letter */
		return TRUE;
	}
#else /* !WIN32 */
	if (path[0] == DIRECTORY_SEPARATOR[0])
	{
		return TRUE;
	}
#endif
	return FALSE;
}

/**
 * Described in header.
 */
bool mkdir_p(const char *path, mode_t mode)
{
	int len;
	char *pos, full[PATH_MAX];
	pos = full;
	if (!path || *path == '\0')
	{
		return TRUE;
	}
	len = snprintf(full, sizeof(full)-1, "%s", path);
	if (len < 0 || len >= sizeof(full)-1)
	{
		DBG1(DBG_LIB, "path string %s too long", path);
		return FALSE;
	}
	/* ensure that the path ends with a '/' */
	if (full[len-1] != '/')
	{
		full[len++] = '/';
		full[len] = '\0';
	}
	/* skip '/' at the beginning */
	while (*pos == '/')
	{
		pos++;
	}
	while ((pos = strchr(pos, '/')))
	{
		*pos = '\0';
		if (access(full, F_OK) < 0)
		{
#ifdef WIN32
			if (_mkdir(full) < 0)
#else
			if (mkdir(full, mode) < 0)
#endif
			{
				DBG1(DBG_LIB, "failed to create directory %s", full);
				return FALSE;
			}
		}
		*pos = '/';
		pos++;
	}
	return TRUE;
}

ENUM(tty_color_names, TTY_RESET, TTY_BG_DEF,
	"\e[0m",
	"\e[1m",
	"\e[4m",
	"\e[5m",
	"\e[30m",
	"\e[31m",
	"\e[32m",
	"\e[33m",
	"\e[34m",
	"\e[35m",
	"\e[36m",
	"\e[37m",
	"\e[39m",
	"\e[40m",
	"\e[41m",
	"\e[42m",
	"\e[43m",
	"\e[44m",
	"\e[45m",
	"\e[46m",
	"\e[47m",
	"\e[49m",
);

/**
 * Get the escape string for a given TTY color, empty string on non-tty FILE
 */
char* tty_escape_get(int fd, tty_escape_t escape)
{
	if (!isatty(fd))
	{
		return "";
	}
	switch (escape)
	{
		case TTY_RESET:
		case TTY_BOLD:
		case TTY_UNDERLINE:
		case TTY_BLINKING:
#ifdef WIN32
			return "";
#endif
		case TTY_FG_BLACK:
		case TTY_FG_RED:
		case TTY_FG_GREEN:
		case TTY_FG_YELLOW:
		case TTY_FG_BLUE:
		case TTY_FG_MAGENTA:
		case TTY_FG_CYAN:
		case TTY_FG_WHITE:
		case TTY_FG_DEF:
		case TTY_BG_BLACK:
		case TTY_BG_RED:
		case TTY_BG_GREEN:
		case TTY_BG_YELLOW:
		case TTY_BG_BLUE:
		case TTY_BG_MAGENTA:
		case TTY_BG_CYAN:
		case TTY_BG_WHITE:
		case TTY_BG_DEF:
			return enum_to_name(tty_color_names, escape);
		/* warn if a escape code is missing */
	}
	return "";
}

#ifndef HAVE_CLOSEFROM
/**
 * Described in header.
 */
void closefrom(int lowfd)
{
	char fd_dir[PATH_MAX];
	int maxfd, fd, len;

	/* try to close only open file descriptors on Linux... */
	len = snprintf(fd_dir, sizeof(fd_dir), "/proc/%u/fd", getpid());
	if (len > 0 && len < sizeof(fd_dir) && access(fd_dir, F_OK) == 0)
	{
		enumerator_t *enumerator = enumerator_create_directory(fd_dir);
		if (enumerator)
		{
			char *rel;
			while (enumerator->enumerate(enumerator, &rel, NULL, NULL))
			{
				fd = atoi(rel);
				if (fd >= lowfd)
				{
					close(fd);
				}
			}
			enumerator->destroy(enumerator);
			return;
		}
	}

	/* ...fall back to closing all fds otherwise */
#ifdef WIN32
	maxfd = _getmaxstdio();
#else
	maxfd = (int)sysconf(_SC_OPEN_MAX);
#endif
	if (maxfd < 0)
	{
		maxfd = 256;
	}
	for (fd = lowfd; fd < maxfd; fd++)
	{
		close(fd);
	}
}
#endif /* HAVE_CLOSEFROM */

/**
 * Return monotonic time
 */
time_t time_monotonic(timeval_t *tv)
{
#ifdef WIN32
	ULONGLONG ms;
	time_t s;

	ms = GetTickCount64();
	s = ms / 1000;
	if (tv)
	{
		tv->tv_sec = s;
		tv->tv_usec = (ms - (s * 1000)) * 1000;
	}
	return s;
#else /* !WIN32 */
#if defined(HAVE_CLOCK_GETTIME) && \
	(defined(HAVE_CONDATTR_CLOCK_MONOTONIC) || \
	 defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC))
	/* as we use time_monotonic() for condvar operations, we use the
	 * monotonic time source only if it is also supported by pthread. */
	timespec_t ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
	{
		if (tv)
		{
			tv->tv_sec = ts.tv_sec;
			tv->tv_usec = ts.tv_nsec / 1000;
		}
		return ts.tv_sec;
	}
#endif /* HAVE_CLOCK_GETTIME && (...) */
	/* Fallback to non-monotonic timestamps:
	 * On MAC OS X, creating monotonic timestamps is rather difficult. We
	 * could use mach_absolute_time() and catch sleep/wakeup notifications.
	 * We stick to the simpler (non-monotonic) gettimeofday() for now.
	 * But keep in mind: we need the same time source here as in condvar! */
	if (!tv)
	{
		return time(NULL);
	}
	if (gettimeofday(tv, NULL) != 0)
	{	/* should actually never fail if passed pointers are valid */
		return -1;
	}
	return tv->tv_sec;
#endif /* !WIN32 */
}

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
 * returns FAILED
 */
status_t return_failed()
{
	return FAILED;
}

/**
 * returns SUCCESS
 */
status_t return_success()
{
	return SUCCESS;
}

/**
 * nop operation
 */
void nop()
{
}

#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)

/**
 * Spinlock for ref_get/put
 */
static spinlock_t *ref_lock;

/**
 * Increase refcount
 */
refcount_t ref_get(refcount_t *ref)
{
	refcount_t current;

	ref_lock->lock(ref_lock);
	current = ++(*ref);
	ref_lock->unlock(ref_lock);

	return current;
}

/**
 * Decrease refcount
 */
bool ref_put(refcount_t *ref)
{
	bool more_refs;

	ref_lock->lock(ref_lock);
	more_refs = --(*ref) > 0;
	ref_lock->unlock(ref_lock);
	return !more_refs;
}

/**
 * Current refcount
 */
refcount_t ref_cur(refcount_t *ref)
{
	refcount_t current;

	ref_lock->lock(ref_lock);
	current = *ref;
	ref_lock->unlock(ref_lock);

	return current;
}

/**
 * Spinlock for all compare and swap operations.
 */
static spinlock_t *cas_lock;

/**
 * Compare and swap if equal to old value
 */
#define _cas_impl(name, type) \
bool cas_##name(type *ptr, type oldval, type newval) \
{ \
	bool swapped; \
	cas_lock->lock(cas_lock); \
	if ((swapped = (*ptr == oldval))) { *ptr = newval; } \
	cas_lock->unlock(cas_lock); \
	return swapped; \
}

_cas_impl(bool, bool)
_cas_impl(ptr, void*)

#endif /* !HAVE_GCC_ATOMIC_OPERATIONS && !HAVE_GCC_SYNC_OPERATIONS */


#ifdef HAVE_FMEMOPEN_FALLBACK

static int fmemread(chunk_t *cookie, char *buf, int size)
{
	int len;

	len = min(size, cookie->len);
	memcpy(buf, cookie->ptr, len);
	*cookie = chunk_skip(*cookie, len);

	return len;
}

static int fmemwrite(chunk_t *cookie, const char *buf, int size)
{
	int len;

	len = min(size, cookie->len);
	memcpy(cookie->ptr, buf, len);
	*cookie = chunk_skip(*cookie, len);

	return len;
}

static int fmemclose(void *cookie)
{
	free(cookie);
	return 0;
}

FILE *fmemopen(void *buf, size_t size, const char *mode)
{
	chunk_t *cookie;

	INIT(cookie,
		.ptr = buf,
		.len = size,
	);

	return funopen(cookie, (void*)fmemread, (void*)fmemwrite, NULL, fmemclose);
}

#endif /* FMEMOPEN fallback*/

/**
 * See header
 */
void utils_init()
{
#ifdef WIN32
	windows_init();
#endif

#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)
	ref_lock = spinlock_create();
	cas_lock = spinlock_create();
#endif

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

#if !defined(HAVE_GCC_ATOMIC_OPERATIONS) && !defined(HAVE_GCC_SYNC_OPERATIONS)
	ref_lock->destroy(ref_lock);
	cas_lock->destroy(cas_lock);
#endif

	strerror_deinit();
}

/**
 * Described in header.
 */
int time_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args)
{
	static const char* months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	time_t *time = *((time_t**)(args[0]));
	bool utc = *((int*)(args[1]));
	struct tm t, *ret = NULL;

	if (*time != UNDEFINED_TIME)
	{
		if (utc)
		{
			ret = gmtime_r(time, &t);
		}
		else
		{
			ret = localtime_r(time, &t);
		}
	}
	if (ret == NULL)
	{
		return print_in_hook(data, "--- -- --:--:--%s----",
							 utc ? " UTC " : " ");
	}
	return print_in_hook(data, "%s %02d %02d:%02d:%02d%s%04d",
						 months[t.tm_mon], t.tm_mday, t.tm_hour, t.tm_min,
						 t.tm_sec, utc ? " UTC " : " ", t.tm_year + 1900);
}

/**
 * Described in header.
 */
int time_delta_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						   const void *const *args)
{
	char* unit = "second";
	time_t *arg1 = *((time_t**)(args[0]));
	time_t *arg2 = *((time_t**)(args[1]));
	u_int64_t delta = llabs(*arg1 - *arg2);

	if (delta > 2 * 60 * 60 * 24)
	{
		delta /= 60 * 60 * 24;
		unit = "day";
	}
	else if (delta > 2 * 60 * 60)
	{
		delta /= 60 * 60;
		unit = "hour";
	}
	else if (delta > 2 * 60)
	{
		delta /= 60;
		unit = "minute";
	}
	return print_in_hook(data, "%" PRIu64 " %s%s", delta, unit,
						 (delta == 1) ? "" : "s");
}

/**
 * Number of bytes per line to dump raw data
 */
#define BYTES_PER_LINE 16

static char hexdig_upper[] = "0123456789ABCDEF";

/**
 * Described in header.
 */
int mem_printf_hook(printf_hook_data_t *data,
					printf_hook_spec_t *spec, const void *const *args)
{
	char *bytes = *((void**)(args[0]));
	u_int len = *((int*)(args[1]));

	char buffer[BYTES_PER_LINE * 3];
	char ascii_buffer[BYTES_PER_LINE + 1];
	char *buffer_pos = buffer;
	char *bytes_pos  = bytes;
	char *bytes_roof = bytes + len;
	int line_start = 0;
	int i = 0;
	int written = 0;

	written += print_in_hook(data, "=> %u bytes @ %p", len, bytes);

	while (bytes_pos < bytes_roof)
	{
		*buffer_pos++ = hexdig_upper[(*bytes_pos >> 4) & 0xF];
		*buffer_pos++ = hexdig_upper[ *bytes_pos       & 0xF];

		ascii_buffer[i++] =
				(*bytes_pos > 31 && *bytes_pos < 127) ? *bytes_pos : '.';

		if (++bytes_pos == bytes_roof || i == BYTES_PER_LINE)
		{
			int padding = 3 * (BYTES_PER_LINE - i);

			while (padding--)
			{
				*buffer_pos++ = ' ';
			}
			*buffer_pos++ = '\0';
			ascii_buffer[i] = '\0';

			written += print_in_hook(data, "\n%4d: %s  %s",
									 line_start, buffer, ascii_buffer);

			buffer_pos = buffer;
			line_start += BYTES_PER_LINE;
			i = 0;
		}
		else
		{
			*buffer_pos++ = ' ';
		}
	}
	return written;
}

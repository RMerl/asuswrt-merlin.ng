/*
 * log.c	Functions in the library call radlib_log() which
 *		does internal logging.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>


#define FR_STRERROR_BUFSIZE (1024)

#ifdef HAVE_THREAD_TLS
/*
 *	GCC on most Linux systems
 */
#define THREAD_TLS __thread

#elif defined(HAVE_DECLSPEC_THREAD)
/*
 *	Visual C++, Borland
 */
#define THREAD_TLS __declspec(thread)
#else

/*
 *	We don't have thread-local storage.  Ensure we don't
 *	ask for it.
 */
#define THREAD_TLS

/*
 *	Use pthread keys if we have pthreads.  For MAC, which should
 *	be very fast.
 */
#ifdef HAVE_PTHREAD_H
#define USE_PTHREAD_FOR_TLS (1)
#endif
#endif

#ifndef USE_PTHREAD_FOR_TLS
/*
 *	Try to create a thread-local-storage version of this buffer.
 */
static THREAD_TLS char fr_strerror_buffer[FR_STRERROR_BUFSIZE];

#else
#include <pthread.h>

static pthread_key_t  fr_strerror_key;
static pthread_once_t fr_strerror_once = PTHREAD_ONCE_INIT;

/* Create Key */
static void fr_strerror_make_key(void)
{
	pthread_key_create(&fr_strerror_key, NULL);
}
#endif

/*
 *	Log to a buffer, trying to be thread-safe.
 */
void fr_strerror_printf(char const *fmt, ...)
{
	va_list ap;

#ifdef USE_PTHREAD_FOR_TLS
	char *buffer;

	pthread_once(&fr_strerror_once, fr_strerror_make_key);

	buffer = pthread_getspecific(fr_strerror_key);
	if (!buffer) {
		int ret;

		buffer = malloc(FR_STRERROR_BUFSIZE);
		if (!buffer) return; /* panic and die! */

		ret = pthread_setspecific(fr_strerror_key, buffer);
		if (ret != 0) {
			fr_perror("Failed recording thread error: %s",
				  strerror(ret));

			return;
		}
	}

	va_start(ap, fmt);
	vsnprintf(buffer, FR_STRERROR_BUFSIZE, fmt, ap);

#else
	va_start(ap, fmt);
	vsnprintf(fr_strerror_buffer, sizeof(fr_strerror_buffer), fmt, ap);
#endif

	va_end(ap);
}

char const *fr_strerror(void)
{
#ifndef USE_PTHREAD_FOR_TLS
	return fr_strerror_buffer;

#else
	char const *msg;

	pthread_once(&fr_strerror_once, fr_strerror_make_key);

	msg = pthread_getspecific(fr_strerror_key);
	if (msg) return msg;

	return "(unknown error)"; /* DON'T return NULL! */
#endif
}

void fr_perror(char const *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	if (strchr(fmt, ':') == NULL)
		fprintf(stderr, ": ");
	fprintf(stderr, "%s\n", fr_strerror());
	va_end(ap);
}

bool fr_assert_cond(char const *file, int line, char const *expr, bool cond)
{
	if (!cond) {
		fr_perror("SOFT ASSERT FAILED %s[%u]: %s", file, line, expr);
		return false;
	}

	return cond;
}

void NEVER_RETURNS _fr_exit(char const *file, int line, int status)
{
#ifndef NDEBUG
	fr_perror("EXIT CALLED %s[%u]: %i", file, line, status);
#endif
	fflush(stderr);

	fr_debug_break();	/* If running under GDB we'll break here */

	exit(status);
}

void NEVER_RETURNS _fr_exit_now(char const *file, int line, int status)
{
#ifndef NDEBUG
	fr_perror("_EXIT CALLED %s[%u]: %i", file, line, status);
#endif
	fflush(stderr);

	fr_debug_break();	/* If running under GDB we'll break here */

	_exit(status);
}

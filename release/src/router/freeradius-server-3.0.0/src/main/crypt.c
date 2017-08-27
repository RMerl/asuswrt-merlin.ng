/*
 * crypt.c	A thread-safe crypt wrapper
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000-2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>

/*
 *  No pthreads, no mutex.
 */
static int fr_crypt_init = 0;
static pthread_mutex_t fr_crypt_mutex;
#endif


/*
 * performs a crypt password check in an thread-safe way.
 *
 * returns:  0 -- check succeeded
 *	  -1 -- failed to crypt
 *	   1 -- check failed
 */
int fr_crypt_check(char const *key, char const *crypted)
{
	char *passwd;
	int cmp = 0;

#ifdef HAVE_PTHREAD_H
	/*
	 *	Ensure we're thread-safe, as crypt() isn't.
	 */
	if (fr_crypt_init == 0) {
		pthread_mutex_init(&fr_crypt_mutex, NULL);
		fr_crypt_init = 1;
	}

	pthread_mutex_lock(&fr_crypt_mutex);
#endif

	passwd = crypt(key, crypted);

	/*
	 *	Got something, check it within the lock.  This is
	 *	faster than copying it to a local buffer, and the
	 *	time spent within the lock is critical.
	 */
	if (passwd) {
		cmp = strcmp(crypted, passwd);
	}

#ifdef HAVE_PTHREAD_H
	pthread_mutex_unlock(&fr_crypt_mutex);
#endif

	/*
	 *	Error.
	 */
	if (!passwd) {
		return -1;
	}

	/*
	 *	OK, return OK.
	 */
	if (cmp == 0) {
		return 0;
	}

	/*
	 *	Comparison failed.
	 */
	return 1;
}

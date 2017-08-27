/*
 * $Id$
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
 * Copyright 2001,2002  Google, Inc.
 * Copyright 2005,2006 TRI-D Systems, Inc.
 */

RCSID("$Id$")

#include "extern.h"

#include <inttypes.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

/** Generate some random bytes
 *
 * @param rnd_data Buffer to write bytes to.
 * @param len Number of bytes to write.
 */
void otp_get_random(uint8_t *rnd_data, size_t len)
{
	size_t bytes_read = 0;
	size_t bytes_left;
	int n;

	while (bytes_read < len) {
		bytes_left = len - bytes_read;
		uint32_t r = fr_rand();

		n = sizeof(r) < bytes_left ? sizeof(r) : bytes_left;

		memcpy(rnd_data + bytes_read, &r, n);

		bytes_read += n;
	}
}

/** Generate a random challenge (ascii chars 0-9)
 *
 * @note This is really cryptocard-specific (automatic ASCII conversion
 * @note and null termination).
 *
 * @param[out] challenge Buffer to write random string to.
 * @param[in] len Number of random bytes to write to buffer.
 */
void otp_async_challenge(char challenge[OTP_MAX_CHALLENGE_LEN + 1],
			 size_t len)
{
	uint8_t rawchallenge[OTP_MAX_CHALLENGE_LEN];
	unsigned int i;

	otp_get_random(rawchallenge, len);

	/* Convert the raw bytes to ASCII decimal. */
	for (i = 0; i < len; ++i) {
		challenge[i] = '0' + rawchallenge[i] % 10;
	}

  	challenge[len] = '\0';
}

/** Guaranteed initialization
 *
 */
void _otp_pthread_mutex_init(pthread_mutex_t *mutexp, pthread_mutexattr_t const *attr, char const *caller)
{
	int rc;

	rc = pthread_mutex_init(mutexp, attr);
	if (rc) {
		ERROR("rlm_otp: %s: pthread_mutex_init: %s",
		       caller, strerror(rc));

		exit(1);
	}
}

/** Guaranteed lock
 *
 */
void _otp_pthread_mutex_lock(pthread_mutex_t *mutexp, char const *caller)
{
	int rc;

	rc = pthread_mutex_lock(mutexp);
	if (rc) {
		ERROR("rlm_otp: %s: pthread_mutex_lock: %s",
		       caller, strerror(rc));

		exit(1);
	}
}

/** Guaranteed trylock
 *
 */
int _otp_pthread_mutex_trylock(pthread_mutex_t *mutexp, char const *caller)
{
	int rc;

	rc = pthread_mutex_trylock(mutexp);
	if (rc && rc != EBUSY) {
		ERROR("rlm_otp: %s: pthread_mutex_trylock: %s",
		       caller, strerror(rc));

		exit(1);
	}

	return rc;
}

/** Guaranteed unlock
 *
 */
void _otp_pthread_mutex_unlock(pthread_mutex_t *mutexp, char const *caller)
{
	int rc;

	rc = pthread_mutex_unlock(mutexp);
  	if (rc) {
		ERROR("rlm_otp: %s: pthread_mutex_unlock: %s",
		       caller, strerror(rc));

		exit(1);
  	}
}

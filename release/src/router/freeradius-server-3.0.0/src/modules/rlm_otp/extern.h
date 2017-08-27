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

#ifndef EXTERN_H
#define EXTERN_H

RCSIDH(extern_h, "$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#include <sys/types.h>
#include <pthread.h>

#include "otp.h"	/* OTP_MAX_CHALLENGE_LEN, otp_pwe_t */

/* otpd rendezvous point */
#define OTP_OTPD_RP "/var/run/otpd/socket"

/* Default prompt for presentation of challenge */
#define OTP_CHALLENGE_PROMPT "Challenge: %{reply:OTP-Challenge}\n Response: "

typedef struct rlm_otp_t {
	char const *name;	//!< Instance name for mod_authorize().
	char *otpd_rp;		//!< Otpd rendezvous point.
	char *chal_prompt;	//!< Text to present challenge to user
				//!< must have %s.

	uint8_t hmac_key[16];   //!< because it doesn't track State

	int challenge_len;	//!< Challenge length, min 5 digits.
	int challenge_delay;	//!< Max delay time for response, in seconds.
	int allow_sync;		//!< Useful to override pwdfile
				//!< card_type settings.
	int allow_async;	//!< C/R mode allowed?

	int mschapv2_mppe_policy;	//!< Whether or not do to mppe for
					//!< mschapv2.
	int mschapv2_mppe_types;	//!< Key type/length for mschapv2/mppe.
	int mschap_mppe_policy;		//!< Whether or not do to mppe for
					//!< mschap .
	int mschap_mppe_types;		//!< key type/length for mschap/mppe.
} rlm_otp_t;

/* otp_mppe.c */
void otp_mppe(REQUEST *, otp_pwe_t, rlm_otp_t const *, char const *);

/* otp_pw_valid.c */
int otp_pw_valid(REQUEST *, int, char const *, rlm_otp_t const *, char []);

/* otp_radstate.c */
#define OTP_MAX_RADSTATE_LEN 2 + (OTP_MAX_CHALLENGE_LEN * 2 + 8 + 8 + 32)*2 + 1

size_t otp_gen_state(char [OTP_MAX_RADSTATE_LEN],
		     char const [OTP_MAX_CHALLENGE_LEN],
		     size_t,
		     int32_t, int32_t, uint8_t const [16]);

/* otp_pwe.c */
extern const	DICT_ATTR *pwattr[8];
void		otp_pwe_init(void);
otp_pwe_t	otp_pwe_present(REQUEST const *);

/* otp_util.c */
void	otp_get_random(uint8_t *, size_t);
void	otp_async_challenge(char[OTP_MAX_CHALLENGE_LEN + 1], size_t);
ssize_t	otp_a2x(uint8_t const *, size_t, uint8_t *);

void	_otp_pthread_mutex_init(pthread_mutex_t *, pthread_mutexattr_t const *,
				char const *);
void	_otp_pthread_mutex_lock(pthread_mutex_t *, char const *);
int	_otp_pthread_mutex_trylock(pthread_mutex_t *, char const *);
void	_otp_pthread_mutex_unlock(pthread_mutex_t *, char const *);

#define otp_pthread_mutex_init(a, b) _otp_pthread_mutex_init((a), (b), __func__)
#define otp_pthread_mutex_lock(a) _otp_pthread_mutex_lock((a), __func__)
#define otp_pthread_mutex_trylock(a) _otp_pthread_mutex_trylock((a), __func__)
#define otp_pthread_mutex_unlock(a) _otp_pthread_mutex_unlock((a), __func__)

#endif /* EXTERN_H */

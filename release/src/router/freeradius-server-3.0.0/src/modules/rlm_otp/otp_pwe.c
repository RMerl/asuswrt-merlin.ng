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

/*
 * This file implements passcode (password) checking functions for each
 * supported encoding (PAP, CHAP, etc.).  The current libradius interface
 * is not sufficient for X9.9 use.
 */

RCSID("$Id$")

/* avoid inclusion of these FR headers which conflict w/ OpenSSL */
#define _FR_MD4_H
#define _FR_SHA1_H
#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include "extern.h"

USES_APPLE_DEPRECATED_API
#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include <string.h>

/* Attribute IDs for supported password encodings. */
#define SIZEOF_PWATTR (4 * 2)
DICT_ATTR const *pwattr[SIZEOF_PWATTR];


/* Initialize the pwattr array for supported password encodings. */
void
otp_pwe_init(void)
{
	DICT_ATTR const *da;

	/*
	 * Setup known password types.  These are pairs.
	 * NB: Increase pwattr array size when adding a type.
	 *     It should be sized as (number of password types * 2)
	 * NB: Array indices must match otp_pwe_t! (see otp.h)
	 */
	(void) memset(pwattr, 0, sizeof(pwattr));

	/* PAP */
	da = dict_attrbyname("User-Password");
	if (da) {
		pwattr[0] = da;
		pwattr[1] = da;
	}

	/* CHAP */
	da = dict_attrbyname("CHAP-Challenge");
	if (da) {
		pwattr[2] = da;

		da = dict_attrbyname("CHAP-Password");
		if (da) {
			pwattr[3] = da;
		} else {
			pwattr[2] = NULL;
		}
	}

#if 0
	/* MS-CHAP (recommended not to use) */
	da = dict_attrbyname("MS-CHAP-Challenge");
	if (da) {
		pwattr[4] = da;

		da = dict_attrbyname("MS-CHAP-Response");
		if (da) {
			pwattr[5] = da;
		} else {
			pwattr[4] = NULL;
		}
	}
#endif /* 0 */

	/* MS-CHAPv2 */
	da = dict_attrbyname("MS-CHAP-Challenge");
	if (da) {
    		pwattr[6] = da;

    		da = dict_attrbyname("MS-CHAP2-Response");
    		if (da) {
			pwattr[7] = da;
    		} else {
      			pwattr[6] = NULL;
      		}
	}
}


/*
 * Test for password presence in an Access-Request packet.
 * Returns 0 for "no supported password present", or the
 * password encoding type.
 */
otp_pwe_t otp_pwe_present(REQUEST const *request)
{
	unsigned i;

	for (i = 0; i < SIZEOF_PWATTR; i += 2) {
		if (!pwattr[i]) {
			continue;
		}

		if (pairfind(request->packet->vps, pwattr[i]->attr,
			     pwattr[i]->vendor, TAG_ANY) &&
		    pairfind(request->packet->vps, pwattr[i + 1]->attr,
			     pwattr[i + 1]->vendor, TAG_ANY)) {
			DEBUG("rlm_otp: %s: password attributes %s, %s",
			      __func__, pwattr[i]->name, pwattr[i + 1]->name);

			return i + 1; /* Can't return 0 (indicates failure) */
		}
	}

	DEBUG("rlm_otp: %s: no password attributes present", __func__);
	return 0;
}

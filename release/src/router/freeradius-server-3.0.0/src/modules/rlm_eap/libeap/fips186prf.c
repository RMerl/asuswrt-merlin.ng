/*
 * fips186prf.c    An implementation of the FIPS-186-2 SHA1-based PRF.
 *
 * The development of the EAP/SIM support was funded by Internet Foundation
 * Austria (http://www.nic.at/ipa).
 *
 * This code was written from scratch by Michael Richardson, and it is
 * dual licensed under both GPL and BSD.
 *
 * Version:     $Id$
 *
 * GPL notice:
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
 * BSD notice:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Copyright 2003  Michael Richardson <mcr@sandelman.ottawa.on.ca>
 * Copyright 2006  The FreeRADIUS server project
 *
 */

RCSID("$Id$")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <freeradius-devel/sha1.h>

/*
 * we do it in 8-bit chunks, because we have to keep the numbers
 * in network byte order (i.e. MSB)
 *
 * make it a structure so that we can do structure assignments.
 */
typedef struct onesixty {
	uint8_t p[20];
} onesixty;

static void onesixty_add_mod(onesixty *sum, onesixty *a, onesixty *b)
{
	uint32_t s;
	int i, carry;

	carry = 0;
	for(i=19; i>=0; i--) {
/*	for(i=0; i<20; i++) {  */
		s = a->p[i] + b->p[i] + carry;
		sum->p[i] = s & 0xff;
		carry = s >> 8;
	}
}

/*
 * run the FIPS-186-2 PRF on the given Master Key (160 bits)
 * in order to derive 1280 bits (160 bytes) of keying data from
 * it.
 *
 * Given that in EAP-SIM, this is coming from a 64-bit Kc it seems
 * like an awful lot of "randomness" to pull out.. (MCR)
 *
 */
void fips186_2prf(uint8_t mk[20], uint8_t finalkey[160])
{
	fr_SHA1_CTX context;
	int j;
	onesixty xval, xkey, w_0, w_1, sum, one;
	uint8_t *f;
	uint8_t zeros[64];

	/*
	 * let XKEY := MK,
	 *
	 * Step 3: For j = 0 to 3 do
	 *   a. XVAL = XKEY
	 *   b. w_0 = SHA1(XVAL)
	 *   c. XKEY = (1 + XKEY + w_0) mod 2^160
	 *   d. XVAL = XKEY
	 *   e. w_1 = SHA1(XVAL)
	 *   f. XKEY = (1 + XKEY + w_1) mod 2^160
	 * 3.3 x_j = w_0|w_1
	 *
	 */
	memcpy(&xkey, mk, sizeof(xkey));

	/* make the value 1 */
	memset(&one,  0, sizeof(one));
	one.p[19]=1;

	f=finalkey;

	for(j=0; j<4; j++) {
		/*   a. XVAL = XKEY  */
		xval = xkey;

		/*   b. w_0 = SHA1(XVAL)  */
		fr_SHA1Init(&context);

		memset(zeros, 0, sizeof(zeros));
		memcpy(zeros, xval.p, 20);
#ifndef WITH_OPENSSL_SHA1
		fr_SHA1Transform(context.state, zeros);
#else
		fr_SHA1Transform(&context, zeros);
#endif
		fr_SHA1FinalNoLen(w_0.p, &context);

		/*   c. XKEY = (1 + XKEY + w_0) mod 2^160 */
		onesixty_add_mod(&sum,  &xkey, &w_0);
		onesixty_add_mod(&xkey, &sum,  &one);

		/*   d. XVAL = XKEY  */
		xval = xkey;

		/*   e. w_1 = SHA1(XVAL)  */
		fr_SHA1Init(&context);

		memset(zeros, 0, sizeof(zeros));
		memcpy(zeros, xval.p, 20);
#ifndef WITH_OPENSSL_SHA1
		fr_SHA1Transform(context.state, zeros);
#else
		fr_SHA1Transform(&context, zeros);
#endif
		fr_SHA1FinalNoLen(w_1.p, &context);

		/*   f. XKEY = (1 + XKEY + w_1) mod 2^160 */
		onesixty_add_mod(&sum,  &xkey, &w_1);
		onesixty_add_mod(&xkey, &sum,  &one);

		/* now store it away */
		memcpy(f, &w_0, 20);
		f += 20;

		memcpy(f, &w_1, 20);
		f += 20;
	}
}

/*
 * test vectors
 * from http://csrc.nist.gov/CryptoToolkit/dss/Examples-1024bit.pdf
 *
 * page 5
 *
 * XKEY=     bd029bbe 7f51960b cf9edb2b 61f06f0f eb5a38b6
 * XSEED=    00000000 00000000 00000000 00000000 00000000
 *
 *
 * The first loop through step 3.2 provides:
 *
 * XVAL=     bd029bbe 7f51960b cf9edb2b 61f06f0f eb5a38b6
 *
 * Using the routine in Appendix 3.3, Constructing The Function G From SHA-1,
 * in step 3.2.b of the Change Notice algorithm for computing values of x
 * provides:
 *
 * w[0]=     2070b322 3dba372f de1c0ffc 7b2e3b49 8b260614
 *
 *
 * The following value is the updated XKEY value from step 3.2.c:
 *
 * XKEY=     dd734ee0 bd0bcd3b adbaeb27 dd1eaa59 76803ecb
 *
 * The second loop through step 3.2 provides:
 *
 * XVAL=     dd734ee0 bd0bcd3b adbaeb27 dd1eaa59 76803ecb
 *
 * Using the routine in Appendix 3.3, Constructing The Function G From SHA-1,
 * in step 3.2.b of the Change Notice algorithm for computing values of x
 * provides:
 *
 * w[1]=     3c6c18ba cb0f6c55 babb1378 8e20d737 a3275116
 *
 * The following value is the updated XKEY value from step 3.2.c:
 *
 *
 * XKEY=     19df679b 881b3991 6875fea0 6b3f8191 19a78fe2
 *
 * Step 3.3 provides the following values:
 *
 * w[0] || w[1]=  2070b322 3dba372f de1c0ffc 7b2e3b49 8b260614
 *		3c6c18ba cb0f6c55 babb1378 8e20d737 a3275116
 *
 */

#ifdef TEST_CASE

#include <assert.h>

uint8_t mk[20]={ 0xbd, 0x02, 0x9b, 0xbe, 0x7f, 0x51, 0x96, 0x0b,
		  0xcf, 0x9e, 0xdb, 0x2b, 0x61, 0xf0, 0x6f, 0x0f,
		  0xeb, 0x5a, 0x38, 0xb6 };

main(int argc, char *argv[])
{
	uint8_t finalkey[160];
	int i, j, k;

	fips186_2prf(mk, finalkey);

	printf("Input was: |");
	j=0;
	for (i = 0; i < 20; i++) {
		if(j==4) {
			printf("_");
			j=0;
		}
		j++;

		printf("%02x", mk[i]);
	}

	printf("|\nOutput was: ");
	j=0; k=0;
	for (i = 0; i < 160; i++) {
		if(k==20) {
			printf("\n	    ");
			k=0;
			j=0;
		}
		if(j==4) {
			printf("_");
			j=0;
		}
		k++;
		j++;

		printf("%02x", finalkey[i]);
	}
	printf("\n");
}
#endif

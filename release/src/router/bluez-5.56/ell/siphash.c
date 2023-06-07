/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
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

#include "siphash-private.h"

/*
 * Based on public domain SipHash reference C implementation
 *
 * Written in 2012 by
 * Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>
 * Daniel J. Bernstein <djb@cr.yp.to>
 *
 */

#define ROTL(x,b) (uint64_t) (((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)			\
	(p)[0] = (uint8_t) ((v));	\
	(p)[1] = (uint8_t) ((v) >>  8);	\
	(p)[2] = (uint8_t) ((v) >> 16);	\
	(p)[3] = (uint8_t) ((v) >> 24);

#define U64TO8_LE(p, v)					\
	U32TO8_LE((p), (uint32_t) ((v)));		\
	U32TO8_LE((p) + 4, (uint32_t) ((v) >> 32));

#define U8TO64_LE(p)			\
	(((uint64_t) ((p)[0])) |	\
	((uint64_t) ((p)[1]) <<  8) |	\
	((uint64_t) ((p)[2]) << 16) |	\
	((uint64_t) ((p)[3]) << 24) |	\
	((uint64_t) ((p)[4]) << 32) |	\
	((uint64_t) ((p)[5]) << 40) |	\
	((uint64_t) ((p)[6]) << 48) |	\
	((uint64_t) ((p)[7]) << 56))

#define SIPROUND				\
	do {					\
		v0 += v1; v1=ROTL(v1, 13);	\
		v1 ^= v0; v0=ROTL(v0, 32);	\
		v2 += v3; v3=ROTL(v3, 16);	\
		v3 ^= v2;			\
		v0 += v3; v3=ROTL(v3, 21);	\
		v3 ^= v0;			\
		v2 += v1; v1=ROTL(v1, 17);	\
		v1 ^= v2; v2=ROTL(v2, 32);	\
	} while(0)

void _siphash24(uint8_t out[8], const uint8_t *in, size_t inlen,
						const uint8_t k[16])
{
	/* "somepseudorandomlygeneratedbytes" */
	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;
	uint64_t b;
	uint64_t k0 = U8TO64_LE(k);
	uint64_t k1 = U8TO64_LE(k + 8);
	uint64_t m;
	const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
	const int left = inlen & 7;

	b = ((uint64_t) inlen) << 56;
	v3 ^= k1;
	v2 ^= k0;
	v1 ^= k1;
	v0 ^= k0;

	for (; in != end; in += 8) {
		m = U8TO64_LE(in);
		v3 ^= m;
		SIPROUND;
		SIPROUND;
		v0 ^= m;
	}

	switch (left) {
	case 7:
		b |= ((uint64_t) in[6]) << 48;
		/* fall through */
	case 6:
		b |= ((uint64_t) in[5]) << 40;
		/* fall through */
	case 5:
		b |= ((uint64_t) in[4]) << 32;
		/* fall through */
	case 4:
		b |= ((uint64_t) in[3]) << 24;
		/* fall through */
	case 3:
		b |= ((uint64_t) in[2]) << 16;
		/* fall through */
	case 2:
		b |= ((uint64_t) in[1]) << 8;
		/* fall through */
	case 1:
		b |= ((uint64_t) in[0]);
		break;
	case 0:
		break;
	}

	v3 ^= b;
	SIPROUND;
	SIPROUND;
	v0 ^= b;
	v2 ^= 0xff;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	b = v0 ^ v1 ^ v2  ^ v3;
	U64TO8_LE(out, b)
}

/* bcm_rand.c
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *   <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <typedefs.h>

/*  Crypto-graphic strength RNG :  the security rests on the key, not on the rand().
 *		Key length should be at least 32 bytes, viewed as 16 short integers. Each integer
 *      of the key is a "one-time pad", because after it is added to the random,
 *      it is XOR'ed by the resulting random.
 */

static  /* lengths are in 16-bit integers */
uint32 seed(uint16* buf, int buf_len, uint16* key, int key_len, int (*rng)(void *ctx))
{
	uint32 k1 = 0;
	int j, n = (buf_len < key_len) ? buf_len : key_len;
	for (j = 0; j < n; j++) {
		buf[j] = rng(&k1) + key[j];
		k1 = (k1 << 1) ^ key[j];
	}
	return k1;
}

void randomize(uint16* buf, int buf_len, uint16* key, int key_len, int (*rng)(void *ctx))
{
	int i, n = 0;
	time_t t;
	uint32 k32 = (uint32)time(&t);

	for (i = 0; i < buf_len; i++) {
		if (i % key_len == 0)
			k32 += seed(buf+i, buf_len-i, key, key_len, rng);

		k32 = (k32 >> 7 | k32 <<25);  /* rotate 7 bits */
		buf[i] ^= k32;
		k32 += rng(&k32);
		buf[i] ^= k32;

		n = (n + 1 + k32 % (key_len-1)) % key_len;	/* round robin */
		buf[i] += key[n];
		if (buf[i] < key[n])
			buf[i]++;			/* 50% chance of carry on every bit */

		key[n] ^= buf[i] ^ (k32 >> n);		/* "messes" up the key */
	}
}

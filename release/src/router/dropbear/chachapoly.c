/*
 * Dropbear SSH
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * Copyright (c) 2020 by Vladislav Grishenko
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include "includes.h"
#include "algo.h"
#include "dbutil.h"
#include "chachapoly.h"

#if DROPBEAR_CHACHA20POLY1305

#define CHACHA20_KEY_LEN 32
#define CHACHA20_BLOCKSIZE 8
#define POLY1305_KEY_LEN 32
#define POLY1305_TAG_LEN 16

static const struct ltc_cipher_descriptor dummy = {.name = NULL};

static const struct dropbear_hash dropbear_chachapoly_mac =
	{NULL, POLY1305_KEY_LEN, POLY1305_TAG_LEN};

const struct dropbear_cipher dropbear_chachapoly =
	{&dummy, CHACHA20_KEY_LEN*2, CHACHA20_BLOCKSIZE};

static int dropbear_chachapoly_start(int UNUSED(cipher), const unsigned char* UNUSED(IV),
			const unsigned char *key, int keylen,
			int UNUSED(num_rounds), dropbear_chachapoly_state *state) {
	int err;

	TRACE2(("enter dropbear_chachapoly_start"))

	if (keylen != CHACHA20_KEY_LEN*2) {
		return CRYPT_ERROR;
	}

	if ((err = chacha_setup(&state->chacha, key,
				CHACHA20_KEY_LEN, 20)) != CRYPT_OK) {
		return err;
	}

	if ((err = chacha_setup(&state->header, key + CHACHA20_KEY_LEN,
				CHACHA20_KEY_LEN, 20) != CRYPT_OK)) {
		return err;
	}

	TRACE2(("leave dropbear_chachapoly_start"))
	return CRYPT_OK;
}

static int dropbear_chachapoly_crypt(unsigned int seq,
			const unsigned char *in, unsigned char *out,
			unsigned long len, unsigned long taglen,
			dropbear_chachapoly_state *state, int direction) {
	poly1305_state poly;
	unsigned char seqbuf[8], key[POLY1305_KEY_LEN], tag[POLY1305_TAG_LEN];
	int err;

	TRACE2(("enter dropbear_chachapoly_crypt"))

	if (len < 4 || taglen != POLY1305_TAG_LEN) {
		return CRYPT_ERROR;
	}

	STORE64H((uint64_t)seq, seqbuf);
	chacha_ivctr64(&state->chacha, seqbuf, sizeof(seqbuf), 0);
	if ((err = chacha_keystream(&state->chacha, key, sizeof(key))) != CRYPT_OK) {
		return err;
	}

	poly1305_init(&poly, key, sizeof(key));
	if (direction == LTC_DECRYPT) {
		poly1305_process(&poly, in, len);
		poly1305_done(&poly, tag, &taglen);
		if (constant_time_memcmp(in + len, tag, taglen) != 0) {
			return CRYPT_ERROR;
		}
	}

	chacha_ivctr64(&state->header, seqbuf, sizeof(seqbuf), 0);
	if ((err = chacha_crypt(&state->header, in, 4, out)) != CRYPT_OK) {
		return err;
	}

	chacha_ivctr64(&state->chacha, seqbuf, sizeof(seqbuf), 1);
	if ((err = chacha_crypt(&state->chacha, in + 4, len - 4, out + 4)) != CRYPT_OK) {
		return err;
	}

	if (direction == LTC_ENCRYPT) {
		poly1305_process(&poly, out, len);
		poly1305_done(&poly, out + len, &taglen);
	}

	TRACE2(("leave dropbear_chachapoly_crypt"))
	return CRYPT_OK;
}

static int dropbear_chachapoly_getlength(unsigned int seq,
			const unsigned char *in, unsigned int *outlen,
			unsigned long len, dropbear_chachapoly_state *state) {
	unsigned char seqbuf[8], buf[4];
	int err;

	TRACE2(("enter dropbear_chachapoly_getlength"))

	if (len < sizeof(buf)) {
		return CRYPT_ERROR;
	}

	STORE64H((uint64_t)seq, seqbuf);
	chacha_ivctr64(&state->header, seqbuf, sizeof(seqbuf), 0);
	if ((err = chacha_crypt(&state->header, in, sizeof(buf), buf)) != CRYPT_OK) {
		return err;
	}

	LOAD32H(*outlen, buf);

	TRACE2(("leave dropbear_chachapoly_getlength"))
	return CRYPT_OK;
}

const struct dropbear_cipher_mode dropbear_mode_chachapoly =
	{(void *)dropbear_chachapoly_start, NULL, NULL,
	 (void *)dropbear_chachapoly_crypt,
	 (void *)dropbear_chachapoly_getlength, &dropbear_chachapoly_mac};

#endif /* DROPBEAR_CHACHA20POLY1305 */

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
#include "gcm.h"

#if DROPBEAR_ENABLE_GCM_MODE

#define GHASH_LEN 16

static const struct dropbear_hash dropbear_ghash =
	{NULL, 0, GHASH_LEN};

static int dropbear_gcm_start(int cipher, const unsigned char *IV,
			const unsigned char *key, int keylen,
			int UNUSED(num_rounds), dropbear_gcm_state *state) {
	int err;

	TRACE2(("enter dropbear_gcm_start"))

	if ((err = gcm_init(&state->gcm, cipher, key, keylen)) != CRYPT_OK) {
		return err;
	}
	memcpy(state->iv, IV, GCM_NONCE_LEN);

	TRACE2(("leave dropbear_gcm_start"))
	return CRYPT_OK;
}

static int dropbear_gcm_crypt(unsigned int UNUSED(seq),
			const unsigned char *in, unsigned char *out,
			unsigned long len, unsigned long taglen,
			dropbear_gcm_state *state, int direction) {
	unsigned char *iv, tag[GHASH_LEN];
	int i, err;

	TRACE2(("enter dropbear_gcm_crypt"))

	if (len < 4 || taglen != GHASH_LEN) {
		return CRYPT_ERROR;
	}

	gcm_reset(&state->gcm);

	if ((err = gcm_add_iv(&state->gcm,
				state->iv, GCM_NONCE_LEN)) != CRYPT_OK) {
		return err;
	}

	if ((err = gcm_add_aad(&state->gcm, in, 4)) != CRYPT_OK) {
		return err;
	}

	if ((err = gcm_process(&state->gcm, (unsigned char *) in + 4,
				len - 4, out + 4, direction)) != CRYPT_OK) {
		return err;
	}

	if (direction == LTC_ENCRYPT) {
		gcm_done(&state->gcm, out + len, &taglen);
	} else {
		gcm_done(&state->gcm, tag, &taglen);
		if (constant_time_memcmp(in + len, tag, taglen) != 0) {
			return CRYPT_ERROR;
		}
	}

	/* increment invocation counter */
	iv = state->iv + GCM_IVFIX_LEN;
	for (i = GCM_IVCTR_LEN - 1; i >= 0 && ++iv[i] == 0; i--);

	TRACE2(("leave dropbear_gcm_crypt"))
	return CRYPT_OK;
}

static int dropbear_gcm_getlength(unsigned int UNUSED(seq),
			const unsigned char *in, unsigned int *outlen,
			unsigned long len, dropbear_gcm_state* UNUSED(state)) {
	TRACE2(("enter dropbear_gcm_getlength"))

	if (len < 4) {
		return CRYPT_ERROR;
	}

	LOAD32H(*outlen, in);

	TRACE2(("leave dropbear_gcm_getlength"))
	return CRYPT_OK;
}

const struct dropbear_cipher_mode dropbear_mode_gcm =
	{(void *)dropbear_gcm_start, NULL, NULL,
	 (void *)dropbear_gcm_crypt,
	 (void *)dropbear_gcm_getlength, &dropbear_ghash};

#endif /* DROPBEAR_ENABLE_GCM_MODE */

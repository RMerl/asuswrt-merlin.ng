/*
 * Copyright (c) 2023 Markus Friedl.  All rights reserved.
 * Copyright (c) 2025 Loganaden Velvindron. All rights reserved.
 * Copyright (c) 2025 Jaykishan Mutkawoa. All rights reserved.
 * Copyright (c) 2025 Keshwarsingh "Kavish" Nadan. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"

#include <sys/types.h>

#include <stdio.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_ENDIAN_H
# include <endian.h>
#endif

#include "kex.h"

#if DROPBEAR_MLKEM768

#include "dbutil.h"
#include "compat.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "libcrux_mlkem768_sha3.h"
#pragma GCC diagnostic pop

#include "mlkem768.h"
#include "dbrandom.h"

int
crypto_kem_mlkem768_keypair(unsigned char *pk, unsigned char *sk)
{
	unsigned char rnd[LIBCRUX_ML_KEM_KEY_PAIR_PRNG_LEN];
	struct libcrux_mlkem768_keypair keypair;

	static_assert(sizeof(keypair.sk.value) == crypto_kem_mlkem768_SECRETKEYBYTES, "len");
	static_assert(sizeof(keypair.pk.value) == crypto_kem_mlkem768_PUBLICKEYBYTES, "len");

	genrandom(rnd, sizeof(rnd));
	keypair = libcrux_ml_kem_mlkem768_portable_generate_key_pair(rnd);
	memcpy(pk, keypair.pk.value, crypto_kem_mlkem768_PUBLICKEYBYTES);
	memcpy(sk, keypair.sk.value, crypto_kem_mlkem768_SECRETKEYBYTES);
	m_burn(rnd, sizeof(rnd));
	m_burn(&keypair, sizeof(keypair));
	return 0;
}

int
crypto_kem_mlkem768_enc(unsigned char *c, unsigned char *k,
const unsigned char *pk)
{
	unsigned char rnd[LIBCRUX_ML_KEM_ENC_PRNG_LEN];
	struct libcrux_mlkem768_enc_result enc;
	struct libcrux_mlkem768_pk mlkem_pub;

	static_assert(sizeof(mlkem_pub.value) == crypto_kem_mlkem768_PUBLICKEYBYTES, "len");
	static_assert(sizeof(enc.fst.value) == crypto_kem_mlkem768_CIPHERTEXTBYTES, "len");
	static_assert(sizeof(enc.snd) == crypto_kem_mlkem768_BYTES, "len");

	memcpy(mlkem_pub.value, pk, crypto_kem_mlkem768_PUBLICKEYBYTES);
	/* generate and encrypt KEM key with client key */
	genrandom(rnd, sizeof(rnd));
	enc = libcrux_ml_kem_mlkem768_portable_encapsulate(&mlkem_pub, rnd);
	memcpy(c, enc.fst.value, sizeof(enc.fst.value));
	memcpy(k, enc.snd, sizeof(enc.snd));

	m_burn(rnd, sizeof(rnd));
	m_burn(&enc, sizeof(enc));
	return 0;
}

int
crypto_kem_mlkem768_dec(unsigned char *k, const unsigned char *c,
const unsigned char *sk)
{
	struct libcrux_mlkem768_sk mlkem_priv;
	struct libcrux_mlkem768_ciphertext mlkem_ciphertext;

	static_assert(sizeof(mlkem_priv.value) == crypto_kem_mlkem768_SECRETKEYBYTES, "len");
	static_assert(sizeof(mlkem_ciphertext.value) == crypto_kem_mlkem768_CIPHERTEXTBYTES, "len");

	memcpy(mlkem_priv.value, sk, crypto_kem_mlkem768_SECRETKEYBYTES);
	memcpy(mlkem_ciphertext.value, c, crypto_kem_mlkem768_CIPHERTEXTBYTES);
	libcrux_ml_kem_mlkem768_portable_decapsulate(&mlkem_priv,
	    &mlkem_ciphertext, k);
	m_burn(&mlkem_priv, sizeof(mlkem_priv));
	m_burn(&mlkem_ciphertext, sizeof(mlkem_ciphertext));
	return 0;
}

#endif /* USE_MLKEM768 */

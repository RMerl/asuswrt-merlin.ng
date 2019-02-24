/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup aesni_key aesni_key
 * @{ @ingroup aesni
 */

#ifndef AESNI_KEY_H_
#define AESNI_KEY_H_

#include <library.h>

#include <wmmintrin.h>

/**
 * AES block size, in bytes
 */
#define AES_BLOCK_SIZE 16

typedef struct aesni_key_t aesni_key_t;

/**
 * Key schedule for encryption/decryption using on AES-NI.
 */
struct aesni_key_t {

	/**
	 * Destroy a aesni_key_t.
	 */
	void (*destroy)(aesni_key_t *this);

	/**
	 * Number of AES rounds (10, 12, 14)
	 */
	int rounds;

	/**
	 * Key schedule, for each round + the round 0 (whitening)
	 */
	__attribute__((aligned(sizeof(__m128i)))) __m128i schedule[];
};

/**
 * Create a AESNI key schedule instance.
 *
 * @param encrypt		TRUE for encryption schedule, FALSE for decryption
 * @param key			non-expanded crypto key, 16, 24 or 32 bytes
 * @return				key schedule, NULL on invalid key size
 */
aesni_key_t *aesni_key_create(bool encrypt, chunk_t key);

#endif /** AESNI_KEY_H_ @}*/

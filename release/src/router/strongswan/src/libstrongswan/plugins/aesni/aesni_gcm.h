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
 * @defgroup aesni_gcm aesni_gcm
 * @{ @ingroup aesni
 */

#ifndef AESNI_GCM_H_
#define AESNI_GCM_H_

#include <library.h>

typedef struct aesni_gcm_t aesni_gcm_t;

/**
 * GCM mode AEAD using AES-NI
 */
struct aesni_gcm_t {

	/**
	 * Implements aead_t interface
	 */
	aead_t aead;
};

/**
 * Create a aesni_gcm instance.
 *
 * @param algo			encryption algorithm, ENCR_AES_GCM*
 * @param key_size		AES key size, in bytes
 * @param salt_size		size of salt value
 * @return				AES-GCM AEAD, NULL if not supported
 */
aesni_gcm_t *aesni_gcm_create(encryption_algorithm_t algo,
							  size_t key_size, size_t salt_size);

#endif /** AESNI_GCM_H_ @}*/

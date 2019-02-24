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
 * @defgroup aesni_ccm aesni_ccm
 * @{ @ingroup aesni
 */

#ifndef AESNI_CCM_H_
#define AESNI_CCM_H_

#include <library.h>

typedef struct aesni_ccm_t aesni_ccm_t;

/**
 * CCM mode AEAD using AES-NI
 */
struct aesni_ccm_t {

	/**
	 * Implements aead_t interface
	 */
	aead_t aead;
};

/**
 * Create a aesni_ccm instance.
 *
 * @param algo			encryption algorithm, ENCR_AES_CCM*
 * @param key_size		AES key size, in bytes
 * @param salt_size		size of salt value
 * @return				AES-CCM AEAD, NULL if not supported
 */
aesni_ccm_t *aesni_ccm_create(encryption_algorithm_t algo,
							  size_t key_size, size_t salt_size);

#endif /** AESNI_CCM_H_ @}*/

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
 * @defgroup aesni_xcbc aesni_xcbc
 * @{ @ingroup aesni
 */

#ifndef AESNI_XCBC_H_
#define AESNI_XCBC_H_

#include <crypto/mac.h>
#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Create a generic mac_t object using AESNI XCBC
 *
 * @param algo		underlying encryption algorithm
 * @param key_size	size of encryption key, in bytes
 */
mac_t *aesni_xcbc_create(encryption_algorithm_t algo, size_t key_size);

/**
 * Creates a new prf_t object based AESNI XCBC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *aesni_xcbc_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on AESNI XCBC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if not supported
 */
signer_t *aesni_xcbc_signer_create(integrity_algorithm_t algo);

#endif /** AESNI_XCBC_H_ @}*/

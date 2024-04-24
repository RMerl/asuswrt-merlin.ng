/*
 * Copyright (C) 2015 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#ifndef AESNI_CMAC_H_
#define AESNI_CMAC_H_

#include <crypto/mac.h>
#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Creates a new prf_t object based AESNI CMAC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *aesni_cmac_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on AESNI CMAC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if  not supported
 */
signer_t *aesni_cmac_signer_create(integrity_algorithm_t algo);

#endif /** AESNI_CMAC_H_ @}*/

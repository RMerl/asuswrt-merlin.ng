/*
 * Copyright (C) 2017 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup tpm_rng tpm_rng
 * @{ @ingroup tpm
 */

#ifndef TPM_RNG_H_
#define TPM_RNG_H_

typedef struct tpm_rng_t tpm_rng_t;

#include <library.h>

/**
 * rng_t implementation via TSS 2.0
 */
struct tpm_rng_t {

	/**
	 * Implements rng_t.
	 */
	rng_t rng;
};

/**
 * Creates a tpm_rng_t instance.
 *
 * @param quality	required quality of randomness
 * @return			created tpm_rng_t
 */
tpm_rng_t *tpm_rng_create(rng_quality_t quality);

#endif /** TPM_RNG_H_ @} */

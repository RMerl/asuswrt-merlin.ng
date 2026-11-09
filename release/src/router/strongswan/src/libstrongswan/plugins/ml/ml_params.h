/*
 * Copyright (C) 2024 Tobias Brunner
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
 * @defgroup ml_params ml_params
 * @{ @ingroup ml_p
 */

#ifndef ML_PARAMS_H_
#define ML_PARAMS_H_

#include <crypto/key_exchange.h>

typedef struct ml_kem_params_t ml_kem_params_t;

/**
 * Constant n used throughout the algorithms.
 */
#define ML_KEM_N 256

/**
 * The prime q = 2^8 * 13 + 1.
 */
#define ML_KEM_Q 3329

/**
 * Length of the seeds and hashes as well as the shared secret.
 */
#define ML_KEM_SEED_LEN 32

/**
 * Length of an enoded polynomial (used for the public key).
 */
#define ML_KEM_POLY_LEN 384

/**
 * Parameters for ML-KEM.
 */
struct ml_kem_params_t {

	/**
	 * Key exchange method.
	 */
	const key_exchange_method_t method;

	/**
	 * Module dimension k.
	 */
	const uint8_t k;

	/**
	 * Factor eta_1 for generating s, e and y.
	 */
	const uint8_t eta1;

	/**
	 * Factor eta_2 for generating e1 and e2.
	 */
	const uint8_t eta2;

	/**
	 * Parameter du for compression/encoding.
	 */
	const uint8_t du;

	/**
	 * Parameter dv for compression/encoding.
	 */
	const uint8_t dv;

	/**
	 * Length of the public key (k * ML_KEM_POLY_LEN + ML_KEM_SEED_LEN).
	 */
	const uint16_t pk_len;

	/**
	 * Length of the ciphertext ((ML_KEM_N * (k * du + kv)) / 8).
	 */
	const uint16_t ct_len;
};

/**
 * Precalculated Zeta^BitRev_7(i) mod q values for NTT (see Appendix A in
 * FIPS 203).  The second half is also used as Zeta^(2*BitRev_7(i)+1) mod q
 * values.
 */
extern const uint16_t ml_kem_zetas[128];

/**
 * Get parameters for a specific ML-KEM method.
 *
 * @param method		key exchange method
 * @return				parameters, NULL if not supported
 */
const ml_kem_params_t *ml_kem_params_get(key_exchange_method_t method);

#endif /** ML_PARAMS_H_ @}*/

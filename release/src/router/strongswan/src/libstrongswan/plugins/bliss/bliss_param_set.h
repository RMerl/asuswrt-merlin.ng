/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup bliss_param_set bliss_param_set
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_PARAM_SET_H_
#define BLISS_PARAM_SET_H_

typedef enum bliss_param_set_id_t bliss_param_set_id_t;
typedef struct bliss_param_set_t bliss_param_set_t;

#include "ntt_fft_params.h"
#include "bliss_huffman_code.h"

#include <library.h>

/**
 * BLISS signature parameter set ID list
 */
enum bliss_param_set_id_t {
	BLISS_I =     1,
	BLISS_II =    2,
	BLISS_III =   3,
	BLISS_IV =    4,
	BLISS_B_I =   5,
	BLISS_B_II =  6,
	BLISS_B_III = 7,
	BLISS_B_IV =  8
};

extern enum_name_t *bliss_param_set_id_names;

/**
 * BLISS
 */
struct bliss_param_set_t {

	/**
	 * BLISS parameter set ID
	 */
	const bliss_param_set_id_t id;

	/**
	 * BLISS parameter set OID
	 */
	const int oid;

	/**
	 * Security strength in bits
	 */
	const uint16_t strength;

	/**
	 * Prime modulus
	 */
	const uint16_t q;

	/**
	 * Number of bits in q
	 */
	const uint16_t q_bits;

	/**
	 * Inverse of (q + 2) mod 2q
	 */
	const uint16_t q2_inv;

	/**
	 * Ring dimension equal to the number of polynomial coefficients
	 */
	const uint16_t n;

	/**
	 * Number of bits in n
	 */
	const uint16_t n_bits;

	/**
	 * FFT parameters
	 */
	const ntt_fft_params_t *fft_params;

	/**
	 * Number of [-1, +1] secret key coefficients
	 */
	const uint16_t non_zero1;

	/**
	 * Number of [-2, +2] secret key coefficients
	 */
	const uint16_t non_zero2;

	/**
	 * Number of secret key terms that go into Nk(S) norm
	 */
	const uint16_t kappa;

	/**
	 * Maximum Nk(S) tolerable NK(S) norm (BLISS only)
	 */
	const uint32_t nks_max;

	/**
	 * Maximum value Pmax for ||Sc'||^2 norm (BLISS-B only)
	 */
	const uint32_t p_max;

	/**
	 * Standard deviation sigma
	 */
	const uint16_t sigma;

	/**
	 *  k_sigma = ceiling[ sqrt(2*ln 2) * sigma ]
	 */
	const uint16_t k_sigma;

	/**
	 *  Number of bits in k_sigma
	 */
	const uint16_t k_sigma_bits;

	/**
	 * Coefficients for Bernoulli sampling with exponential biases
	 */
	const uint8_t *c;

	/**
	 * Number of columns in Bernoulli coefficient table
	 */
	const size_t c_cols;

	/**
	 * Number of rows in Bernoulli coefficient table
	 */
	const size_t c_rows;

	/**
	 * Number of bits in z1
	 */
	const uint16_t z1_bits;

	/**
	 * Number of z2 bits to be dropped after rounding
	 */
	const uint16_t d;

	/**
	 * Modulus p = floor(2q / 2^d) applied after bit dropping
	 */
	const uint16_t p;

	/**
	 * M = sigma^2 / alpha_rejection^2
	 */
	const uint32_t M;

	/**
	 * B_infinity bound
	 */
	const uint16_t B_inf;

	/**
	 * B_verify bound
	 */
	const uint32_t B_l2;

};

/**
 * Get BLISS signature parameter set by BLISS parameter set ID
 *
 * @param id	BLISS parameter set ID
 * @return		BLISS parameter set
*/
const bliss_param_set_t* bliss_param_set_get_by_id(bliss_param_set_id_t id);

/**
 * Get BLISS signature parameter set by BLISS parameter set OID
 *
 * @param oid	BLISS parameter set OID
 * @return		BLISS parameter set
*/
const bliss_param_set_t* bliss_param_set_get_by_oid(int oid);

#endif /** BLISS_PARAM_SET_H_ @}*/

/*
 * Copyright (C) 2014-2016 Andreas Steffen
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
 * @defgroup ntru_poly ntru_poly
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_POLY_H_
#define NTRU_POLY_H_

typedef struct ntru_poly_t ntru_poly_t;

#include <library.h>
#include <crypto/xofs/xof.h>

/**
 * Implements a trinary polynomial storing the indices of non-zero coefficients 
 */
struct ntru_poly_t {

	/**
	 * Get the size of the indices array
	 *
	 * @return			number of indices
	 */
	size_t (*get_size)(ntru_poly_t *this);

	/**
	 * @return			array containing the indices of the non-zero coefficients
	 */
	uint16_t* (*get_indices)(ntru_poly_t *this);

	/**
	 * @param array		array containing all N coefficients of the polynomial
	 */
	void (*get_array)(ntru_poly_t *this, uint16_t *array);

	/**
	 * Multiply polynomial a with ntru_poly_t object b having sparse coefficients
	 * to form result polynomial c = a * b
	 *
	 * @param a			input polynomial a
	 * @param b			output polynomial c
	 */
	void (*ring_mult)(ntru_poly_t *this, uint16_t *a, uint16_t *c);

	/**
	 * Destroy ntru_poly_t object
	 */
	void (*destroy)(ntru_poly_t *this);
};

/**
 * Create a trits polynomial from a seed using MGF1
 *
 * @param alg				MGF1 algorithm used(XOF_MGF1_SHA1 or XOF_MGF_SHA256)
 * @param seed				seed used by MGF1 to generate trits from
 * @param N					ring dimension, number of polynomial coefficients
 * @param q					large modulus
 * @param c_bits			number of bits for candidate index
 * @param indices_len_p		number of indices for +1 coefficients
 * @param indices_len_m		number of indices for -1 coefficients
 * @param is_product_form	generate multiple polynomials
 */
ntru_poly_t *ntru_poly_create_from_seed(ext_out_function_t alg,	chunk_t seed,
										uint8_t c_bits, uint16_t N, uint16_t q,
										uint32_t indices_len_p,
										uint32_t indices_len_m,
										bool is_product_form);

/**
 * Create a trits polynomial from an array of indices of non-zero coefficients
 *
 * @param data				array of indices of non-zero coefficients
 * @param N					ring dimension, number of polynomial coefficients
 * @param q					large modulus
 * @param indices_len_p		number of indices for +1 coefficients
 * @param indices_len_m		number of indices for -1 coefficients
 * @param is_product_form	generate multiple polynomials
 */
ntru_poly_t *ntru_poly_create_from_data(uint16_t *data, uint16_t N, uint16_t q,
										uint32_t indices_len_p,
										uint32_t indices_len_m,
										bool is_product_form);

#endif /** NTRU_POLY_H_ @}*/


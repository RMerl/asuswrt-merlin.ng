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
 * @defgroup bliss_utils bliss_utils
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_UTILS_H_
#define BLISS_UTILS_H_

#include "bliss_param_set.h"

#include <library.h>

/**
 * Compute the scalar product of two vectors of size n
 *
 * @param x			input vector of size n
 * @param y			input vector of size n
 * @param n			size of input vectors x and y
 * @result			scalar product of x and y
 */
int32_t bliss_utils_scalar_product(int32_t *x, int32_t *y, int n);

/**
 * Drop d bits but round first
 *
 * @param set		BLISS parameter set
 * @param x			input vector x of size n
 * @param xd		rounded vector x with d bits dropped
 */
void bliss_utils_round_and_drop(const bliss_param_set_t *set,
								int32_t *x, int16_t *xd);

/**
 * Generate the binary challenge vector c as an array of kappa indices
 *
 * @param alg			XOF to be used for the internal oracle
 * @param data_hash		hash of the data to be signed
 * @param ud			input vector ud of size n
 * @param set			BLISS parameter set to be used (n, n_bits, kappa)
 * @param c_indices		indexes of non-zero challenge coefficients
 */
bool bliss_utils_generate_c(ext_out_function_t alg, chunk_t data_hash,
							uint16_t *ud, const bliss_param_set_t *set,
							uint16_t *c_indices);

/**
 * Check the infinity and l2 norms of the vectors z1 and z2d << d
 *
 * @param set		BLISS parameter set
 * @param z1		input vector
 * @param z2d		input vector
 * @result			TRUE if infinite and l2 norms do not exceed boundaries
 */
bool bliss_utils_check_norms(const bliss_param_set_t *set,
							 int32_t *z1, int16_t *z2d);

#endif /** BLISS_UTILS_H_ @}*/

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
 * @defgroup ml_poly ml_poly
 * @{ @ingroup ml_p
 */

#ifndef ML_POLY_H_
#define ML_POLY_H_

#include "ml_params.h"

typedef struct ml_poly_t ml_poly_t;

/**
 * Represents an element in R_q = Z_q[X]/(X^n + 1) i.e. a polynomial of the
 * form f[0] + f[1]*X + ... + f[n-1]*X^n-1.
 */
struct ml_poly_t {

	/**
	 * Coefficients of the polynomial.
	 */
	uint16_t f[ML_KEM_N];
};

/**
 * Add two polynomials (a + b mod q).
 *
 * @param a		polynomial a
 * @param b		polynomial b
 * @param res	result of adding a and b (can be one of the others)
 */
void ml_poly_add(ml_poly_t *a, ml_poly_t *b, ml_poly_t *res);

/**
 * Add polynomials in array a and b (a[i] + b[i] mod q for i in 0 to k-1).
 *
 * @param k		number of polynomials in each array
 * @param a		array of polynomials a
 * @param b		array of polynomials b
 * @param res	array of resulting polynomials (can be one of the others)
 */
void ml_poly_add_arr(u_int k, ml_poly_t *a, ml_poly_t *b, ml_poly_t *res);

/**
 * Subtract a polynomial from another (a - b mod q).
 *
 * @param a		polynomial a
 * @param b		polynomial b
 * @param res	result of subtracting b from a (can be one of the others)
 */
void ml_poly_sub(ml_poly_t *a, ml_poly_t *b, ml_poly_t *res);

#endif /** ML_POLY_H_ @}*/

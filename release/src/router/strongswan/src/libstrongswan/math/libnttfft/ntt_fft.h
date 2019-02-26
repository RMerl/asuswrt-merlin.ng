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
 * @defgroup ntt_p libnttfft
 *
 * @defgroup ntt_fft ntt_fft
 * @{ @ingroup ntt_p
 */

#ifndef NTT_FFT_H_
#define NTT_FFT_H_

#include "ntt_fft_params.h"

#include <library.h>

typedef struct ntt_fft_t ntt_fft_t;

/**
 * Implements a Number Theoretic Transform (NTT) via the FFT algorithm
 */
struct ntt_fft_t {

	/**
	 * Get the size of the Number Theoretic Transform
	 *
	 * @result			Transform size
	 */
	uint16_t (*get_size)(ntt_fft_t *this);

	/**
	 * Get the prime modulus of the Number Theoretic Transform
	 *
	 * @result			Prime modulus
	 */
	uint16_t (*get_modulus)(ntt_fft_t *this);

	/**
	 * Compute the [inverse] NTT of a polynomial
	 *
	 * @param a			Coefficient of input polynomial
	 * @param b			Coefficient of output polynomial
	 * @param inverse	TRUE if the inverse NTT has to be computed
	 */
	void (*transform)(ntt_fft_t *this, uint32_t *a, uint32_t *b, bool inverse);

	/**
	 * Destroy ntt_fft_t object
	 */
	void (*destroy)(ntt_fft_t *this);
};

/**
 * Create a ntt_fft_t object for a given FFT parameter set
 *
 * @param params		FFT parameters
 */
ntt_fft_t *ntt_fft_create(const ntt_fft_params_t *params);

/**
 * Dummy libnttfft initialization function needed for integrity test
 */
void libnttfft_init(void);


#endif /** NTT_FFT_H_ @}*/

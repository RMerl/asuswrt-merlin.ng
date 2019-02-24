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
 * @defgroup ntt_fft_params ntt_fft_params
 * @{ @ingroup ntt_p
 */

#ifndef NTT_FFT_PARAMS_H_
#define NTT_FFT_PARAMS_H_

#include <library.h>

typedef struct ntt_fft_params_t ntt_fft_params_t;

/**
 * Defines the parameters for an NTT computed via the FFT algorithm
 */
struct ntt_fft_params_t {

	/**
	 * Prime modulus
	 */
	uint16_t q;

	/**
	 * Inverse of Prime modulus (-q_inv * q mod r = 1)
	 */
	uint16_t q_inv;

	/**
	 * Logarithm of Montgomery radix: log2(r)
	 */
	uint16_t rlog;

	/**
	 * Square of Montgomery radix: r^2 mod q
	 */
	const uint32_t r2;

	/**
	 * Montgomery radix mask: (1<<rlog) - 1
	 */
	const uint32_t rmask;

	/**
	 * Size of the FFT with the condition k * n = q-1
	 */
	const uint16_t n;

	/**
	 * Inverse of n mod q used for normalization of the FFT
	 */
	const uint16_t n_inv;

	/**
	 * Number of FFT stages  stages = log2(n)
	 */
	const uint16_t stages;

	/**
	 * FFT twiddle factors (n-th roots of unity) in Montgomery form
	 */
	const uint16_t *wr;

	/**
	 * FFT phase shift (2n-th roots of unity) in forward transform
	 */
	const uint16_t *wf;

	/**
	 * FFT phase shift (2n-th roots of unity) and scaling in inverse transform
	 */
	const uint16_t *wi;

	/**
	 * Subsampling of FFT twiddle factors table
	 */
	const uint16_t s;

	/**
	 * FFT bit reversal
	 */
	const uint16_t *rev;

};

/**
 * FFT parameters for q = 12289 and n = 1024
 */
extern const ntt_fft_params_t ntt_fft_12289_1024;

/**
 * FFT parameters for q = 12289 and n = 512
 */
extern const ntt_fft_params_t ntt_fft_12289_512;

/**
 * FFT parameters for q = 17 and n = 8
 */
extern const ntt_fft_params_t ntt_fft_17_8;

#endif /** NTT_FFT_PARAMS_H_ @}*/

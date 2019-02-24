/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup tls_prf tls_prf
 * @{ @ingroup libtls
 */

#ifndef TLS_PRF_H_
#define TLS_PRF_H_

typedef struct tls_prf_t tls_prf_t;

#include <crypto/prfs/prf.h>

/**
 * The PRF function specified on TLS, based on HMAC.
 */
struct tls_prf_t {

	/**
	 * Set the key of the PRF function.
	 *
	 * @param key		key to set
	 * @return			TRUE if key set successfully
	 */
	bool (*set_key)(tls_prf_t *this, chunk_t key);

	/**
	 * Generate a series of bytes using a label and a seed.
	 *
	 * @param label		ASCII input label
	 * @param seed		seed input value
	 * @param bytes		number of bytes to get
	 * @param out		buffer receiving bytes
	 * @return			TRUE if bytes generated successfully
	 */
	bool (*get_bytes)(tls_prf_t *this, char *label, chunk_t seed,
					  size_t bytes, char *out);

	/**
	 * Destroy a tls_prf_t.
	 */
	void (*destroy)(tls_prf_t *this);
};

/**
 * Create a tls_prf instance with specific algorithm as in TLS 1.2.
 *
 * @param prf			underlying PRF function to use
 * @return				TLS PRF algorithm
 */
tls_prf_t *tls_prf_create_12(pseudo_random_function_t prf);

/**
 * Create a tls_prf instance with XOred SHA1/MD5 as in TLS 1.0/1.1.
 *
 * @return				TLS PRF algorithm
 */
tls_prf_t *tls_prf_create_10();

#endif /** TLS_PRF_H_ @}*/

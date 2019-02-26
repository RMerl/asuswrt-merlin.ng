/*
 * Copyright (C) 2016 Andreas Steffen
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
n */

/**
 * @defgroup sha3_keccak sha3_keccak
 * @{ @ingroup sha3_p
 */

#ifndef SHA3_KECCAK_H_
#define SHA3_KECCAK_H_

typedef struct sha3_keccak_t sha3_keccak_t;

#include <crypto/hashers/hasher.h>

/**
 * Implements the Keccak-f[1600] sponge function as defined by FIPS-202.
 */
struct sha3_keccak_t {

	/**
	 * Get the available rate in bytes
	 *
	 * @return			rate in bytes
	 */
	u_int (*get_rate)(sha3_keccak_t *this);

	/**
	 * Resets the internal Keccak state
	 */
	void (*reset)(sha3_keccak_t *this);

	/**
	 * Absorbs data into the Keccak state
	 *
	 * @param data		data to be absorbed
	 */
	void (*absorb)(sha3_keccak_t *this, chunk_t data);

	/**
	 * Finalize the absorption phase and switch to the squeeze phase
	 */
	void (*finalize)(sha3_keccak_t *this);

	/**
	 * Squeeze the Keccak state to get output data
	 * Can be called multiple times
	 *
	 * @param out_len	number of output bytes requested
	 * @param out		output buffer, must comprise at least out_len bytes
	 */
	void (*squeeze)(sha3_keccak_t *this, size_t out_len, uint8_t *out);

	/**
	 * Destroy the sha3_keccak_t object
	 */
	void (*destroy)(sha3_keccak_t *this);

};

/**
 * Creates a new sha3_keccak_t.
 *
 * @param	capacity		required capacity to achieve a given security level
 * @param delimited_suffix	bits delimiting the input message
 * @return					sha3_keccak_t object, NULL if capacity too big
 */
sha3_keccak_t *sha3_keccak_create(u_int capacity, uint8_t delimited_suffix);

#endif /** SHA3_KECCAK_H_ @}*/

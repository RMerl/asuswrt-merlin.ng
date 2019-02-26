/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup curve25519_drv curve25519_drv
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_DRV_H_
#define CURVE25519_DRV_H_

typedef struct curve25519_drv_t curve25519_drv_t;

#include <library.h>

/**
 * Private key size of Curve25519
 */
#define CURVE25519_KEY_SIZE 32

/**
 * Backend driver abstraction for Curve25519.
 */
struct curve25519_drv_t {

	/**
	 * Set the private key.
	 *
	 * @param key		32 byte private key, clamped
	 * @return			TRUE if key set
	 */
	bool (*set_key)(curve25519_drv_t *this, u_char *key);

	/**
	 * Calculate Curve25519 for the set key.
	 *
	 * @param in		input data, 32 bytes
	 * @param out		output data, 32 bytes
	 * @return			TRUE if calculated
	 */
	bool (*curve25519)(curve25519_drv_t *this, u_char *in, u_char *out);

	/**
	 * Destroy a curve25519_drv_t.
	 */
	void (*destroy)(curve25519_drv_t *this);
};

/**
 * Create a curve25519_drv instance.
 */
curve25519_drv_t *curve25519_drv_probe();

#endif /** CURVE25519_DRV_H_ @}*/

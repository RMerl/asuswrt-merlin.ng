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
 */

/**
 * @defgroup sha3_shake sha3_shake
 * @{ @ingroup sha3_p
 */

#ifndef SHA3_SHAKE_H_
#define SHA3_SHAKE_H_

typedef struct sha3_shake_t sha3_shake_t;

#include <crypto/xofs/xof.h>

/**
 * Implementation of xof_t interface using the SHA-3 XOF algorithm family
 * SHAKE128 and SHAKE256 as defined by FIPS-202.
 */
struct sha3_shake_t {

	/**
	 * Generic xof_t interface for this Extended Output Function (XOF).
	 */
	xof_t xof_interface;
};

/**
 * Creates a new sha3_shake_t.
 *
 * @param	algorithm	XOF_SHAKE_128 or XOF_SHAKE_256
 * @return				sha3_shake_t object, NULL if not supported
 */
sha3_shake_t* sha3_shake_create(ext_out_function_t algorithm);

#endif /** SHA3_SHAKE_H_ @}*/

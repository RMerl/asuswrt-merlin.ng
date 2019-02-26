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
 * @defgroup chapoly_xof chapoly_xof
 * @{ @ingroup chapoly
 */

#ifndef CHAPOLY_XOF_H_
#define CHAPOLY_XOF_H_

#include <crypto/aead.h>

typedef struct chapoly_xof_t chapoly_xof_t;

/**
 * ChaCha20 XOF implementation
 *
 * Based on RFC 7539 ChaCha20 stream initialized with block counter = 1
 */
struct chapoly_xof_t {

	/**
	 * Generic xof_t interface for this Extended Output Function (XOF).
	 */
	xof_t xof_interface;
};

/**
 * Create a chapoly_xof instance.
 *
 * @param algorithm		XOF_CHACHA20
 * @return				chapoly_xof_t object, NULL if not supported
 */
chapoly_xof_t *chapoly_xof_create(ext_out_function_t algorithm);

#endif /** CHAPOLY_XOF_H_ @}*/

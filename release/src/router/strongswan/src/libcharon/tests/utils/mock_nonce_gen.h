/*
 * Copyright (C) 2016 Tobias Brunner
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
 * Special nonce generator that sets the first byte of the generated nonces to
 * a fixed specified value.
 *
 * @defgroup mock_nonce_gen mock_nonce_gen
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_NONCE_GEN_H_
#define MOCK_NONCE_GEN_H_

#include <crypto/nonce_gen.h>

/**
 * Creates a nonce_gen_t instance.
 *
 * @param first		first byte to set in generated nonces
 * @return			created object
 */
nonce_gen_t *mock_nonce_gen_create(u_char first);

#endif /** MOCK_NONCE_GEN_H_ @} */

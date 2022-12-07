/*
 * Copyright (C) 2016 Tobias Brunner
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
 * Provides a DH implementation that does no real work to make the tests run
 * faster.
 *
 * @defgroup mock_dh mock_dh
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_DH_H_
#define MOCK_DH_H_

#include <crypto/key_exchange.h>

/**
 * Creates a key_exchange_t object.
 *
 * @param method		key_exchange method, supports MODP_NULL only
 * @return				created object
 */
key_exchange_t *mock_dh_create(key_exchange_method_t method);

#endif /** MOCK_DH_H_ @}*/

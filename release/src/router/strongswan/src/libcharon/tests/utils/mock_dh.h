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
 * Provides a DH implementation that does no real work to make the tests run
 * faster.
 *
 * @defgroup mock_dh mock_dh
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_DH_H_
#define MOCK_DH_H_

#include <crypto/diffie_hellman.h>

/**
 * Creates a diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group, supports MODP_NULL only
 * @return				created object
 */
diffie_hellman_t *mock_dh_create(diffie_hellman_group_t group);

#endif /** MOCK_DH_H_ @}*/

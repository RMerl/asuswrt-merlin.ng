/*
 * Copyright (C) 2013 Andreas Steffen
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
 * rng_t providing NIST SP 800-90A entropy test vectors
 *
 * @defgroup test_rng test_rng
 * @{ @ingroup test_utils
 */

#ifndef TEST_RNG_H_
#define TEST_RNG_H_

#include <library.h>

/**
 * Creates a test_rng_t instance.
 *
 * @param entropy	entropy test vector
 * @return			created test_rng_t
 */
rng_t *test_rng_create(chunk_t entropy);

#endif /** TEST_RNG_H_ @} */

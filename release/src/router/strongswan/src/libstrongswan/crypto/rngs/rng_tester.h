/*
 * Copyright (C) 2013-2019 Andreas Steffen
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
 * rng_t providing deterministic output (DO NOT USE IN PRODUCTIVE SYSTEMS!!!)
 *
 * @defgroup rng_tester rng_tester
 * @{ @ingroup crypto
 */

#ifndef RNG_TESTER_H_
#define RNG_TESTER_H_

#include <library.h>

/**
 * Creates a rng_tester_t instance.
 *
 * @param entropy	deterministic output
 * @return			created rng_tester_t
 */
rng_t *rng_tester_create(chunk_t entropy);

#endif /** RNG_TESTER_H_ @} */

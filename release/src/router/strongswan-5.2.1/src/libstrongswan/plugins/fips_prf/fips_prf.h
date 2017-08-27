/*
 * Copyright (C) 2006-2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup fips_prf fips_prf
 * @{ @ingroup fips_prf_p
 */

#ifndef FIPS_PRF_H_
#define FIPS_PRF_H_

typedef struct fips_prf_t fips_prf_t;

#include <library.h>
#include <crypto/prfs/prf.h>
#include <crypto/hashers/hasher.h>

/**
 * Implementation of prf_t using the FIPS 186-2-change1 standard.
 *
 * FIPS defines a "General Purpose Random Number Generator" (Revised
 * Algorithm for Computing m values of x (Appendix 3.1 of FIPS 186-2)). This
 * implementation is not intended for private key generation and therefore does
 * not include the "mod q" operation (see FIPS 186-2-change1 p74).
 * The FIPS PRF is stateful; the key changes every time when bytes are acquired.
 */
struct fips_prf_t {

	/**
	 * Generic prf_t interface for this fips_prf_t class.
	 */
	prf_t prf_interface;
};

/**
 * Creates a new fips_prf_t object.
 *
 * FIPS 186-2 defines G() functions used in the PRF function. It can
 * be implemented either based on SHA1 or DES.
 * The G() function is selected using the algo parameter.
 *
 * @param algo		specific FIPS PRF implementation, specifies G() function
 * @return			fips_prf_t object, NULL if not supported.
 */
fips_prf_t *fips_prf_create(pseudo_random_function_t algo);

#endif /** FIPS_PRF_H_ @}*/

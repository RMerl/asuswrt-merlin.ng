/*
 * Copyright (C) 2012 Tobias Brunner
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
 * Cipher-based Message Authentication Code (CMAC).
 *
 * This class implements the message authentication algorithm
 * described in RFC 4493.
 *
 * @defgroup cmac cmac
 * @{ @ingroup cmac_p
 */

#ifndef CMAC_H_
#define CMAC_H_

#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Creates a new prf_t object based on a CMAC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *cmac_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on a CMAC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if  not supported
 */
signer_t *cmac_signer_create(integrity_algorithm_t algo);

#endif /** CMAC_H_ @}*/

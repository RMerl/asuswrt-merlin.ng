/*
 * Copyright (C) 2012 Tobias Brunner
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
 * Implements the message authentication algorithm described in RFC2104.
 *
 * It uses a hash function, which must be implemented as a hasher_t class.
 *
 * @defgroup hmac_mac mac
 * @{ @ingroup hmac_p
 */

#ifndef HMAC_H_
#define HMAC_H_

#include <crypto/prfs/prf.h>
#include <crypto/signers/signer.h>

/**
 * Creates a new prf_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			prf_t object, NULL if not supported
 */
prf_t *hmac_prf_create(pseudo_random_function_t algo);

/**
 * Creates a new signer_t object based on an HMAC.
 *
 * @param algo		algorithm to implement
 * @return			signer_t, NULL if not supported
 */
signer_t *hmac_signer_create(integrity_algorithm_t algo);

#endif /** HMAC_H_ @}*/

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
 * @defgroup mac_signer mac_signer
 * @{ @ingroup crypto
 */

#ifndef MAC_SIGNER_H_
#define MAC_SIGNER_H_

typedef struct mac_signer_t mac_signer_t;

#include <crypto/mac.h>
#include <crypto/signers/signer.h>

/**
 * Creates an implementation of the signer_t interface using the provided mac_t
 * implementation and truncation length.
 *
 * @note len will be set to mac_t.get_mac_size() if it is greater than that.
 *
 * @param mac		mac_t implementation
 * @param len		length of resulting signature
 * @return			mac_signer_t
 */
signer_t *mac_signer_create(mac_t *mac, size_t len);

#endif /** MAC_SIGNER_H_ @}*/

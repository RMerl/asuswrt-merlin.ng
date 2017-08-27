/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup pubkey_v1_authenticator pubkey_v1_authenticator
 * @{ @ingroup authenticators_v1
 */

#ifndef PUBKEY_V1_AUTHENTICATOR_H_
#define PUBKEY_V1_AUTHENTICATOR_H_

typedef struct pubkey_v1_authenticator_t pubkey_v1_authenticator_t;

#include <sa/authenticator.h>

/**
 * Implementation of authenticator_t using public keys for IKEv1.
 */
struct pubkey_v1_authenticator_t {

	/**
	 * Implemented authenticator_t interface.
	 */
	authenticator_t authenticator;
};

/**
 * Create an authenticator to build and verify public key signatures.
 *
 * @param ike_sa			associated IKE_SA
 * @param initiator			TRUE if we are IKE_SA initiator
 * @param dh				diffie hellman key exchange
 * @param dh_value			others public diffie hellman value
 * @param sa_payload		generated SA payload data, without payload header
 * @param id_payload		encoded ID payload of peer to authenticate or verify
 *							without payload header (gets owned)
 * @param type				key type to use, KEY_RSA or KEY_ECDSA
 * @return					pubkey authenticator
 */
pubkey_v1_authenticator_t *pubkey_v1_authenticator_create(ike_sa_t *ike_sa,
										bool initiator, diffie_hellman_t *dh,
										chunk_t dh_value, chunk_t sa_payload,
										chunk_t id_payload, key_type_t type);

#endif /** PUBKEY_V1_AUTHENTICATOR_H_ @}*/

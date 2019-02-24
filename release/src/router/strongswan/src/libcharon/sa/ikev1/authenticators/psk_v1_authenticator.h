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
 * @defgroup psk_v1_authenticator psk_v1_authenticator
 * @{ @ingroup authenticators_v1
 */

#ifndef PSK_V1_AUTHENTICATOR_H_
#define PSK_V1_AUTHENTICATOR_H_

typedef struct psk_v1_authenticator_t psk_v1_authenticator_t;

#include <sa/authenticator.h>

/**
 * Implementation of authenticator_t using pre-shared keys for IKEv1.
 */
struct psk_v1_authenticator_t {

	/**
	 * Implemented authenticator_t interface.
	 */
	authenticator_t authenticator;
};

/**
 * Create an authenticator to build PSK signatures.
 *
 * @param ike_sa			associated IKE_SA
 * @param initiator			TRUE if we are the IKE_SA initiator
 * @param dh				diffie hellman key exchange
 * @param dh_value			others public diffie hellman value
 * @param sa_payload		generated SA payload data, without payload header
 * @param id_payload		encoded ID payload of peer to authenticate or verify
 *							without payload header (gets owned)
 * @param hybrid			TRUE if used for hybrid authentication without PSK
 * @return					PSK authenticator
 */
psk_v1_authenticator_t *psk_v1_authenticator_create(ike_sa_t *ike_sa,
										bool initiator, diffie_hellman_t *dh,
										chunk_t dh_value, chunk_t sa_payload,
										chunk_t id_payload, bool hybrid);

#endif /** PSK_V1_AUTHENTICATOR_H_ @}*/

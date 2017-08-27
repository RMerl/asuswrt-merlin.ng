/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2006-2009 Martin Willi
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
 * @defgroup pubkey_authenticator pubkey_authenticator
 * @{ @ingroup authenticators_v2
 */

#ifndef PUBKEY_AUTHENTICATOR_H_
#define PUBKEY_AUTHENTICATOR_H_

typedef struct pubkey_authenticator_t pubkey_authenticator_t;

#include <sa/authenticator.h>

/**
 * Implementation of authenticator_t using public key authenitcation.
 */
struct pubkey_authenticator_t {

	/**
	 * Implemented authenticator_t interface.
	 */
	authenticator_t authenticator;
};

/**
 * Create an authenticator to build public key signatures.
 *
 * @param ike_sa			associated ike_sa
 * @param received_nonce	nonce received in IKE_SA_INIT
 * @param sent_init			sent IKE_SA_INIT message data
 * @param reserved			reserved bytes of ID payload
 * @return					public key authenticator
 */
pubkey_authenticator_t *pubkey_authenticator_create_builder(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_init,
									char reserved[3]);

/**
 * Create an authenticator to verify public key signatures.
 *
 * @param ike_sa			associated ike_sa
 * @param sent_nonce		nonce sent in IKE_SA_INIT
 * @param received_init		received IKE_SA_INIT message data
 * @param reserved			reserved bytes of ID payload
 * @return					public key authenticator
 */
pubkey_authenticator_t *pubkey_authenticator_create_verifier(ike_sa_t *ike_sa,
									chunk_t sent_nonce, chunk_t received_init,
									char reserved[3]);

#endif /** PUBKEY_AUTHENTICATOR_H_ @}*/

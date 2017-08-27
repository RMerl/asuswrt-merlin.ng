/*
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
 * @defgroup eap_authenticator eap_authenticator
 * @{ @ingroup authenticators_v2
 */

#ifndef EAP_AUTHENTICATOR_H_
#define EAP_AUTHENTICATOR_H_

typedef struct eap_authenticator_t eap_authenticator_t;

#include <sa/authenticator.h>

/**
 * Implementation of authenticator_t using EAP authentication.
 *
 * Authentication using EAP involves the most complex authenticator. It stays
 * alive over multiple ike_auth transactions and handles multiple EAP
 * messages.
 *
 * @verbatim
                          ike_sa_init
                   ------------------------->
                   <-------------------------
                 followed by multiple ike_auth:

     +--------+                                +--------+
     |  EAP   |       IDi, [IDr,] SA, TS       |  EAP   |
     | client |  --------------------------->  | server |
     |        |          ID, AUTH, EAP         |        |
     |        |  <---------------------------  |        |
     |        |              EAP               |        |
     |        |  --------------------------->  |        |
     |        |              EAP               |        |
     |        |  <---------------------------  |        |
     |        |              EAP               |        |
     |        |  --------------------------->  |        |
     |        |           EAP(SUCCESS)         |        |
     |        |  <---------------------------  |        |
     |        |              AUTH              |        |  If EAP establishes
     |        |  --------------------------->  |        |  a session key, AUTH
     |        |          AUTH, SA, TS          |        |  payloads use this
     |        |  <---------------------------  |        |  key, not SK_pi/pr
     +--------+                                +--------+

   @endverbatim
 */
struct eap_authenticator_t {

	/**
	 * Implemented authenticator_t interface.
	 */
	authenticator_t authenticator;
};

/**
 * Create an authenticator to authenticate against an EAP server.
 *
 * @param ike_sa			associated ike_sa
 * @param received_nonce	nonce received in IKE_SA_INIT
 * @param sent_nonce		nonce sent in IKE_SA_INIT
 * @param received_init		received IKE_SA_INIT message data
 * @param sent_init			sent IKE_SA_INIT message data
 * @param reserved			reserved bytes of ID payload
 * @return					EAP authenticator
 */
eap_authenticator_t *eap_authenticator_create_builder(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3]);

/**
 * Create an authenticator to authenticate EAP clients.
 *
 * @param ike_sa			associated ike_sa
 * @param received_nonce	nonce received in IKE_SA_INIT
 * @param sent_nonce		nonce sent in IKE_SA_INIT
 * @param received_init		received IKE_SA_INIT message data
 * @param sent_init			sent IKE_SA_INIT message data
 * @param reserved			reserved bytes of ID payload
 * @return					EAP authenticator
 */
eap_authenticator_t *eap_authenticator_create_verifier(ike_sa_t *ike_sa,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3]);

#endif /** EAP_AUTHENTICATOR_H_ @}*/

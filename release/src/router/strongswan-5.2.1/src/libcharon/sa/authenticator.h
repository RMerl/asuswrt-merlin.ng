/*
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup authenticator authenticator
 * @{ @ingroup sa
 */

#ifndef AUTHENTICATOR_H_
#define AUTHENTICATOR_H_

typedef enum auth_method_t auth_method_t;
typedef struct authenticator_t authenticator_t;

#include <library.h>
#include <credentials/auth_cfg.h>
#include <sa/ike_sa.h>

/**
 * Method to use for authentication, as defined in IKEv2.
 */
enum auth_method_t {

	/**
	 * No authentication used.
	 */
	AUTH_NONE = 0,

	/**
	 * Computed as specified in section 2.15 of RFC using
	 * an RSA private key over a PKCS#1 padded hash.
	 */
	AUTH_RSA = 1,

	/**
	 * Computed as specified in section 2.15 of RFC using the
	 * shared key associated with the identity in the ID payload
	 * and the negotiated prf function
	 */
	AUTH_PSK = 2,

	/**
	 * Computed as specified in section 2.15 of RFC using a
	 * DSS private key over a SHA-1 hash.
	 */
	AUTH_DSS = 3,

	/**
	 * ECDSA with SHA-256 on the P-256 curve as specified in RFC 4754
	 */
	AUTH_ECDSA_256 = 9,

	/**
	 * ECDSA with SHA-384 on the P-384 curve as specified in RFC 4754
	 */
	AUTH_ECDSA_384 = 10,

	/**
	 * ECDSA with SHA-512 on the P-521 curve as specified in RFC 4754
	 */
	AUTH_ECDSA_521 = 11,

	/**
	 * Generic Secure Password Authentication Method as specified in RFC 6467
	 */
	AUTH_GSPM = 12,

	/**
	 * IKEv1 initiator XAUTH with PSK, outside of IANA range
	 */
	AUTH_XAUTH_INIT_PSK = 256,

	/**
	 * IKEv1 responder XAUTH with PSK, outside of IANA range
	 */
	AUTH_XAUTH_RESP_PSK,

	/**
	 * IKEv1 initiator XAUTH with RSA, outside of IANA range
	 */
	AUTH_XAUTH_INIT_RSA,

	/**
	 * IKEv1 responder XAUTH with RSA, outside of IANA range
	 */
	AUTH_XAUTH_RESP_RSA,

	/**
	 * IKEv1 initiator XAUTH, responder RSA, outside of IANA range
	 */
	AUTH_HYBRID_INIT_RSA,

	/**
	 * IKEv1 responder XAUTH, initiator RSA, outside of IANA range
	 */
	AUTH_HYBRID_RESP_RSA,
};

/**
 * enum names for auth_method_t.
 */
extern enum_name_t *auth_method_names;

/**
 * Authenticator interface implemented by the various authenticators.
 *
 * An authenticator implementation handles AUTH and EAP payloads. Received
 * messages are passed to the process() method, to send authentication data
 * the message is passed to the build() method.
 */
struct authenticator_t {

	/**
	 * Process an incoming message using the authenticator.
	 *
	 * @param message		message containing authentication payloads
	 * @return
	 *						- SUCCESS if authentication successful
	 *						- FAILED if authentication failed
	 *						- NEED_MORE if another exchange required
	 */
	status_t (*process)(authenticator_t *this, message_t *message);

	/**
	 * Attach authentication data to an outgoing message.
	 *
	 * @param message		message to add authentication data to
	 * @return
	 *						- SUCCESS if authentication successful
	 *						- FAILED if authentication failed
	 *						- NEED_MORE if another exchange required
	 */
	status_t (*build)(authenticator_t *this, message_t *message);

	/**
	 * Check if the authenticator is capable of mutual authentication.
	 *
	 * Some authenticator authenticate both peers, e.g. EAP. To support
	 * mutual authentication with only a single authenticator (EAP-only
	 * authentication), it must be mutual. This method is invoked in ike_auth
	 * to check if the given authenticator is capable of doing so.
	 */
	bool (*is_mutual)(authenticator_t *this);

	/**
	 * Destroy authenticator instance.
	 */
	void (*destroy) (authenticator_t *this);
};

/**
 * Create an IKEv2 authenticator to build signatures.
 *
 * @param ike_sa			associated ike_sa
 * @param cfg				authentication configuration
 * @param received_nonce	nonce received in IKE_SA_INIT
 * @param sent_nonce		nonce sent in IKE_SA_INIT
 * @param received_init		received IKE_SA_INIT message data
 * @param sent_init			sent IKE_SA_INIT message data
 * @param reserved			reserved bytes of the ID payload
 * @return					authenticator, NULL if not supported
 */
authenticator_t *authenticator_create_builder(
									ike_sa_t *ike_sa, auth_cfg_t *cfg,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3]);

/**
 * Create an IKEv2 authenticator to verify signatures.
 *
 * @param ike_sa			associated ike_sa
 * @param message			message containing authentication data
 * @param received_nonce	nonce received in IKE_SA_INIT
 * @param sent_nonce		nonce sent in IKE_SA_INIT
 * @param received_init		received IKE_SA_INIT message data
 * @param sent_init			sent IKE_SA_INIT message data
 * @param reserved			reserved bytes of the ID payload
 * @return					authenticator, NULL if not supported
 */
authenticator_t *authenticator_create_verifier(
									ike_sa_t *ike_sa, message_t *message,
									chunk_t received_nonce, chunk_t sent_nonce,
									chunk_t received_init, chunk_t sent_init,
									char reserved[3]);

/**
 * Create an IKEv1 authenticator to build and verify signatures or hash
 * payloads.
 *
 * @note Due to the fixed ID, these authenticators can only be used in one
 * direction at a time.
 *
 * @param ike_sa			associated IKE_SA
 * @param initiator			TRUE if we are the IKE_SA initiator
 * @param auth_method		negotiated authentication method to use
 * @param dh				diffie hellman key exchange
 * @param dh_value			others public diffie hellman value
 * @param sa_payload		generated SA payload data, without payload header
 * @param id_payload		encoded ID payload of peer to authenticate or verify
 *							without payload header (gets owned)
 * @return					authenticator, NULL if not supported
 */
authenticator_t *authenticator_create_v1(ike_sa_t *ike_sa, bool initiator,
								auth_method_t auth_method, diffie_hellman_t *dh,
								chunk_t dh_value, chunk_t sa_payload,
								chunk_t id_payload);

#endif /** AUTHENTICATOR_H_ @}*/

/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup phase1 phase1
 * @{ @ingroup ikev1
 */

#ifndef PHASE1_H_
#define PHASE1_H_

typedef struct phase1_t phase1_t;

#include <sa/ike_sa.h>
#include <crypto/diffie_hellman.h>

/**
 * Common phase 1 helper for main and aggressive mode.
 */
struct phase1_t {

	/**
	 * Create keymat hasher.
	 *
	 * @return				TRUE if hasher created
	 */
	bool (*create_hasher)(phase1_t *this);

	/**
	 * Create DH object using SA keymat.
	 *
	 * @param group			negotiated DH group
	 * @return				TRUE if group supported
	 */
	bool (*create_dh)(phase1_t *this, diffie_hellman_group_t group);

	/**
	 * Derive key material.
	 *
	 * @param peer_cfg		peer config to look up shared key for, or NULL
	 * @param method		negotiated authenticated method
	 * @return				TRUE if successful
	 */
	bool (*derive_keys)(phase1_t *this, peer_cfg_t *peer_cfg,
						auth_method_t method);
	/**
	 * Verify a HASH or SIG payload in message.
	 *
	 * @param method		negotiated auth method
	 * @param message		message containing HASH or SIG payload
	 * @param id_data		encoded identity, including protocol/port fields
	 * @return				TRUE if verified successfully
	 */
	bool (*verify_auth)(phase1_t *this, auth_method_t method,
						message_t *message, chunk_t id_data);

	/**
	 * Build a HASH or SIG payload and add it to message.
	 *
	 * @param method		negotiated auth method
	 * @param message		message to add payload to
	 * @param id_data		encoded identity, including protocol/port fields
	 * @return				TRUE if built successfully
	 */
	bool (*build_auth)(phase1_t *this, auth_method_t method,
					   message_t *message, chunk_t id_data);

	/**
	 * Get the IKEv1 authentication method defined by peer config.
	 *
	 * @param peer_cfg		peer config to get auth method from
	 * @return				auth method, or AUTH_NONE
	 */
	auth_method_t (*get_auth_method)(phase1_t *this, peer_cfg_t *peer_cfg);

	/**
	 * Select a peer config as responder.
	 *
	 * If called after the first successful call the next alternative config
	 * is returned, if any.
	 *
	 * @param method		used authentication method
	 * @param aggressive	TRUE to get an aggressive mode config
	 * @param id			initiator identity
	 * @return				selected peer config, NULL if none found
	 */
	peer_cfg_t* (*select_config)(phase1_t *this, auth_method_t method,
								 bool aggressive, identification_t *id);

	/**
	 * Get configured identity from peer config.
	 *
	 * @param peer_cfg		peer config to get identity from
	 * @param local			TRUE to get own identity, FALSE for remote
	 * @return				identity, pointing to internal config data
	 */
	identification_t* (*get_id)(phase1_t *this, peer_cfg_t *peer_cfg, bool local);

	/**
	 * Check if peer config has virtual IPs pool assigned.
	 *
	 * @param peer_cfg		peer_config to check
	 * @return				TRUE if peer config contains at least one pool
	 */
	bool (*has_pool)(phase1_t *this, peer_cfg_t *peer_cfg);

	/**
	 * Check if peer config has virtual IPs to request
	 *
	 * @param peer_cfg		peer_config to check
	 * @return				TRUE if peer config contains at least one virtual IP
	 */
	bool (*has_virtual_ip)(phase1_t *this, peer_cfg_t *peer_cfg);

	/**
	 * Extract and store SA payload bytes from encoded message.
	 *
	 * @param message		message to extract SA payload bytes from
	 * @return				TRUE if SA payload found
	 */
	bool (*save_sa_payload)(phase1_t *this, message_t *message);

	/**
	 * Add Nonce and KE payload to message.
	 *
	 * @param message		message to add payloads
	 * @return				TRUE if payloads added successfully
	 */
	bool (*add_nonce_ke)(phase1_t *this, message_t *message);

	/**
	 * Extract Nonce and KE payload from message.
	 *
	 * @param message		message to get payloads from
	 * @return				TRUE if payloads extracted successfully
	 */
	bool (*get_nonce_ke)(phase1_t *this, message_t *message);

	/**
	 * Destroy a phase1_t.
	 */
	void (*destroy)(phase1_t *this);
};

/**
 * Create a phase1 instance.
 *
 * @param ike_sa		IKE_SA to set up
 * @param initiator		TRUE if initiating actively
 * @return				Phase 1 helper
 */
phase1_t *phase1_create(ike_sa_t *ike_sa, bool initiator);

#endif /** PHASE1_H_ @}*/

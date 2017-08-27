/*
 * Copyright (C) 2011-2014 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup listener listener
 * @{ @ingroup listeners
 */

#ifndef LISTENER_H_
#define LISTENER_H_

typedef struct listener_t listener_t;

#include <bus/bus.h>

/**
 * Listener interface, listens to events if registered to the bus.
 */
struct listener_t {

	/**
	 * Hook called if a critical alert is raised.
	 *
	 * @param ike_sa	IKE_SA associated to the alert, if any
	 * @param alert		kind of alert
	 * @param ...		alert specific argument list
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*alert)(listener_t *this, ike_sa_t *ike_sa,
				  alert_t alert, va_list args);

	/**
	 * Handle state changes in an IKE_SA.
	 *
	 * @param ike_sa	IKE_SA which changes its state
	 * @param state		new IKE_SA state this IKE_SA changes to
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_state_change)(listener_t *this, ike_sa_t *ike_sa,
							 ike_sa_state_t state);

	/**
	 * Handle state changes in a CHILD_SA.
	 *
	 * @param ike_sa	IKE_SA containing the affected CHILD_SA
	 * @param child_sa	CHILD_SA which changes its state
	 * @param state		new CHILD_SA state this CHILD_SA changes to
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*child_state_change)(listener_t *this, ike_sa_t *ike_sa,
							   child_sa_t *child_sa, child_sa_state_t state);

	/**
	 * Hook called for received/sent messages of an IKE_SA.
	 *
	 * The hook is invoked twice for each message: Once with plain, parsed data
	 * and once encoded and encrypted.
	 *
	 * @param ike_sa	IKE_SA sending/receiving a message
	 * @param message	message object
	 * @param incoming	TRUE for incoming messages, FALSE for outgoing
	 * @param plain		TRUE if message is parsed and decrypted, FALSE it not
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*message)(listener_t *this, ike_sa_t *ike_sa, message_t *message,
					bool incoming, bool plain);

	/**
	 * Hook called with IKE_SA key material.
	 *
	 * @param ike_sa	IKE_SA this keymat belongs to
	 * @param dh		diffie hellman shared secret
	 * @param dh_other	others DH public value (IKEv1 only)
	 * @param nonce_i	initiators nonce
	 * @param nonce_r	responders nonce
	 * @param rekey		IKE_SA we are rekeying, if any (IKEv2 only)
	 * @param shared	shared key used for key derivation (IKEv1-PSK only)
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_keys)(listener_t *this, ike_sa_t *ike_sa, diffie_hellman_t *dh,
					 chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
					 ike_sa_t *rekey, shared_key_t *shared);

	/**
	 * Hook called with CHILD_SA key material.
	 *
	 * @param ike_sa	IKE_SA the child sa belongs to
	 * @param child_sa	CHILD_SA this keymat is used for
	 * @param initiator	initiator of the CREATE_CHILD_SA exchange
	 * @param dh		diffie hellman shared secret
	 * @param nonce_i	initiators nonce
	 * @param nonce_r	responders nonce
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*child_keys)(listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
					   bool initiator, diffie_hellman_t *dh,
					   chunk_t nonce_i, chunk_t nonce_r);

	/**
	 * Hook called if an IKE_SA gets up or down.
	 *
	 * @param ike_sa	IKE_SA coming up/going down
	 * @param up		TRUE for an up event, FALSE for a down event
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_updown)(listener_t *this, ike_sa_t *ike_sa, bool up);

	/**
	 * Hook called when an IKE_SA gets rekeyed.
	 *
	 * @param old		rekeyed IKE_SA getting obsolete
	 * @param new		new IKE_SA replacing old
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_rekey)(listener_t *this, ike_sa_t *old, ike_sa_t *new);

	/**
	 * Hook called when an initiator reestablishes an IKE_SA.
	 *
	 * This is invoked right after creating the new IKE_SA and setting the
	 * peer_cfg (and the old hosts), but before resolving the hosts anew.
	 * It is not invoked on the responder.
	 *
	 * @param old		IKE_SA getting reestablished (is destroyed)
	 * @param new		new IKE_SA replacing old (gets established)
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_reestablish_pre)(listener_t *this, ike_sa_t *old, ike_sa_t *new);

	/**
	 * Hook called when an initiator reestablishes an IKE_SA.
	 *
	 * This is invoked right before the new IKE_SA is checked in after
	 * initiating it.  It is not invoked on the responder.
	 *
	 * @param old		IKE_SA getting reestablished (is destroyed)
	 * @param new		new IKE_SA replacing old (gets established)
	 * @param initiated TRUE if initiation was successful, FALSE otherwise
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*ike_reestablish_post)(listener_t *this, ike_sa_t *old,
								 ike_sa_t *new, bool initiated);

	/**
	 * Hook called when a CHILD_SA gets up or down.
	 *
	 * @param ike_sa	IKE_SA containing the handled CHILD_SA
	 * @param child_sa	CHILD_SA coming up/going down
	 * @param up		TRUE for an up event, FALSE for a down event
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*child_updown)(listener_t *this, ike_sa_t *ike_sa,
						 child_sa_t *child_sa, bool up);

	/**
	 * Hook called when an CHILD_SA gets rekeyed.
	 *
	 * @param ike_sa	IKE_SA containing the rekeyed CHILD_SA
	 * @param old		rekeyed CHILD_SA getting obsolete
	 * @param new		new CHILD_SA replacing old
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*child_rekey)(listener_t *this, ike_sa_t *ike_sa,
						child_sa_t *old, child_sa_t *new);

	/**
	 * Hook called to invoke additional authorization rules.
	 *
	 * An authorization hook gets invoked several times: After each
	 * authentication round, the hook gets invoked with with final = FALSE.
	 * After authentication is complete and the peer configuration is selected,
	 * it is invoked again, but with final = TRUE.
	 *
	 * @param ike_sa	IKE_SA to authorize
	 * @param final		TRUE if this is the final hook invocation
	 * @param success	set to TRUE to complete IKE_SA, FALSE abort
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*authorize)(listener_t *this, ike_sa_t *ike_sa,
					  bool final, bool *success);

	/**
	 * CHILD_SA traffic selector narrowing hook.
	 *
	 * This hook is invoked for each CHILD_SA and allows plugins to modify
	 * the traffic selector list negotiated for this CHILD_SA.
	 *
	 * @param ike_sa	IKE_SA the created CHILD_SA is created in
	 * @param child_sa	CHILD_SA set up with these traffic selectors
	 * @param type		type of hook getting invoked
	 * @param local		list of local traffic selectors to narrow
	 * @param remote	list of remote traffic selectors to narrow
	 */
	bool (*narrow)(listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
				narrow_hook_t type, linked_list_t *local, linked_list_t *remote);

	/**
	 * Virtual IP address assignment hook.
	 *
	 * This hook gets invoked after virtual IPs have been assigned to a peer
	 * for a specific IKE_SA, and again before they get released.
	 *
	 * @param ike_sa	IKE_SA the VIPs are assigned to
	 * @param assign	TRUE if assigned to IKE_SA, FALSE if released
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*assign_vips)(listener_t *this, ike_sa_t *ike_sa, bool assign);

	/**
	 * Virtual IP and configuration attribute handler hook.
	 *
	 * This hook gets invoked after virtual IP and other configuration
	 * attributes just got installed or are about to get uninstalled on a peer
	 * receiving them.
	 *
	 * @param ike_sa	IKE_SA the VIPs/attributes are handled on
	 * @param handle	TRUE if handled by IKE_SA, FALSE on release
	 * @return			TRUE to stay registered, FALSE to unregister
	 */
	bool (*handle_vips)(listener_t *this, ike_sa_t *ike_sa, bool handle);
};

#endif /** LISTENER_H_ @}*/

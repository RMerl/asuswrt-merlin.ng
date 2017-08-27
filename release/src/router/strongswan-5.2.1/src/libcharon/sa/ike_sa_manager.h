/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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
 * @defgroup ike_sa_manager ike_sa_manager
 * @{ @ingroup sa
 */

#ifndef IKE_SA_MANAGER_H_
#define IKE_SA_MANAGER_H_

typedef struct ike_sa_manager_t ike_sa_manager_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <encoding/message.h>
#include <config/peer_cfg.h>

/**
 * Manages and synchronizes access to all IKE_SAs.
 *
 * To synchronize access to thread-unsave IKE_SAs, they are checked out for
 * use and checked in afterwards. A checked out SA is exclusively accessible
 * by the owning thread.
 */
struct ike_sa_manager_t {

	/**
	 * Checkout an existing IKE_SA.
	 *
	 * @param ike_sa_id			the SA identifier, will be updated
	 * @returns
	 * 							- checked out IKE_SA if found
	 * 							- NULL, if specified IKE_SA is not found.
	 */
	ike_sa_t* (*checkout) (ike_sa_manager_t* this, ike_sa_id_t *sa_id);

	/**
	 * Create and check out a new IKE_SA.
	 *
	 * @param version			IKE version of this SA
	 * @param initiator			TRUE for initiator, FALSE otherwise
	 * @returns 				created and checked out IKE_SA
	 */
	ike_sa_t* (*checkout_new) (ike_sa_manager_t* this, ike_version_t version,
							   bool initiator);

	/**
	 * Checkout an IKE_SA by a message.
	 *
	 * In some situations, it is necessary that the manager knows the
	 * message to use for the checkout. This has the following reasons:
	 *
	 * 1. If the targeted IKE_SA is already processing a message, we do not
	 *    check it out if the message ID is the same.
	 * 2. If it is an IKE_SA_INIT request, we have to check if it is a
	 *    retransmission. If so, we have to drop the message, we would
	 *    create another unneeded IKE_SA for each retransmitted packet.
	 *
	 * A call to checkout_by_message() returns a (maybe new created) IKE_SA.
	 * If processing the message does not make sense (for the reasons above),
	 * NULL is returned.
	 *
	 * @param ike_sa_id			the SA identifier, will be updated
	 * @returns
	 * 							- checked out/created IKE_SA
	 * 							- NULL to not process message further
	 */
	ike_sa_t* (*checkout_by_message) (ike_sa_manager_t* this, message_t *message);

	/**
	 * Checkout an IKE_SA for initiation by a peer_config.
	 *
	 * To initiate, a CHILD_SA may be established within an existing IKE_SA.
	 * This call checks for an existing IKE_SA by comparing the configuration.
	 * If the CHILD_SA can be created in an existing IKE_SA, the matching SA
	 * is returned.
	 * If no IKE_SA is found, a new one is created. This is also the case when
	 * the found IKE_SA is in the DELETING state.
	 *
	 * @param peer_cfg			configuration used to find an existing IKE_SA
	 * @return					checked out/created IKE_SA
	 */
	ike_sa_t* (*checkout_by_config) (ike_sa_manager_t* this,
									 peer_cfg_t *peer_cfg);

	/**
	 * Check for duplicates of the given IKE_SA.
	 *
	 * Measures are taken according to the uniqueness policy of the IKE_SA.
	 * The return value indicates whether duplicates have been found and if
	 * further measures should be taken (e.g. cancelling an IKE_AUTH exchange).
	 * check_uniqueness() must be called before the IKE_SA is complete,
	 * deadlocks occur otherwise.
	 *
	 * @param ike_sa			ike_sa to check
	 * @param force_replace		replace existing SAs, regardless of unique policy
	 * @return					TRUE, if the given IKE_SA has duplicates and
	 * 							should be deleted
	 */
	bool (*check_uniqueness)(ike_sa_manager_t *this, ike_sa_t *ike_sa,
							 bool force_replace);

	/**
	 * Check if we already have a connected IKE_SA between two identities.
	 *
	 * @param me				own identity
	 * @param other				remote identity
	 * @param family			address family to include in uniqueness check
	 * @return					TRUE if we have a connected IKE_SA
	 */
	bool (*has_contact)(ike_sa_manager_t *this, identification_t *me,
						identification_t *other, int family);

	/**
	 * Check out an IKE_SA a unique ID.
	 *
	 * Every IKE_SA and every CHILD_SA is uniquely identified by an ID.
	 * These checkout function uses, depending
	 * on the child parameter, the unique ID of the IKE_SA or the reqid
	 * of one of a IKE_SAs CHILD_SA.
	 *
	 * @param id				unique ID of the object
	 * @param child				TRUE to use CHILD, FALSE to use IKE_SA
	 * @return
	 * 							- checked out IKE_SA, if found
	 * 							- NULL, if not found
	 */
	ike_sa_t* (*checkout_by_id) (ike_sa_manager_t* this, u_int32_t id,
								 bool child);

	/**
	 * Check out an IKE_SA by the policy/connection name.
	 *
	 * Check out the IKE_SA by the configuration name, either from the IKE- or
	 * one of its CHILD_SAs.
	 *
	 * @param name				name of the connection/policy
	 * @param child				TRUE to use policy name, FALSE to use conn name
	 * @return
	 * 							- checked out IKE_SA, if found
	 * 							- NULL, if not found
	 */
	ike_sa_t* (*checkout_by_name) (ike_sa_manager_t* this, char *name,
								   bool child);

	/**
	 * Create an enumerator over all stored IKE_SAs.
	 *
	 * While enumerating an IKE_SA, it is temporarily checked out and
	 * automatically checked in after the current enumeration step.
	 *
	 * @param wait				TRUE to wait for checked out SAs, FALSE to skip
	 * @return					enumerator over all IKE_SAs.
	 */
	enumerator_t *(*create_enumerator) (ike_sa_manager_t* this, bool wait);

	/**
	 * Create an enumerator over ike_sa_id_t*, matching peer identities.
	 *
	 * The remote peer is identified by its XAuth or EAP identity, if available.
	 *
	 * @param me				local peer identity to match
	 * @param other				remote peer identity to match
	 * @param family			address family to match, 0 for any
	 * @return					enumerator over ike_sa_id_t*
	 */
	enumerator_t* (*create_id_enumerator)(ike_sa_manager_t *this,
								identification_t *me, identification_t *other,
								int family);

	/**
	 * Checkin the SA after usage.
	 *
	 * If the IKE_SA is not registered in the manager, a new entry is created.
	 *
	 * @param ike_sa_id			the SA identifier, will be updated
	 * @param ike_sa			checked out SA
	 */
	void (*checkin) (ike_sa_manager_t* this, ike_sa_t *ike_sa);

	/**
	 * Destroy a checked out SA.
	 *
	 * The IKE SA is destroyed without notification of the remote peer.
	 * Use this only if the other peer doesn't respond or behaves not
	 * as predicted.
	 * Checking in and destruction is an atomic operation (for the IKE_SA),
	 * so this can be called if the SA is in a "unclean" state, without the
	 * risk that another thread can get the SA.
	 *
	 * @param ike_sa			SA to delete
	 */
	void (*checkin_and_destroy) (ike_sa_manager_t* this, ike_sa_t *ike_sa);

	/**
	 * Get the number of IKE_SAs currently registered.
	 *
	 * @return					number of registered IKE_SAs
	 */
	u_int (*get_count)(ike_sa_manager_t *this);

	/**
	 * Get the number of IKE_SAs which are in the connecting state.
	 *
	 * To prevent the server from resource exhaustion, cookies and other
	 * mechanisms are used. The number of half open IKE_SAs is a good
	 * indicator to see if a peer is flooding the server.
	 * If a host is supplied, only the number of half open IKE_SAs initiated
	 * from this IP are counted.
	 * Only SAs for which we are the responder are counted.
	 *
	 * @param ip				NULL for all, IP for half open IKE_SAs with IP
	 * @return					number of half open IKE_SAs
	 */
	u_int (*get_half_open_count) (ike_sa_manager_t *this, host_t *ip);

	/**
	 * Delete all existing IKE_SAs and destroy them immediately.
	 *
	 * Threads will be driven out, so all SAs can be deleted cleanly.
	 * To a flush(), an immediate call to destroy() is mandatory; no other
	 * method may be used.
	 */
	void (*flush)(ike_sa_manager_t *this);

	/**
	 * Destroys the manager with all associated SAs.
	 *
	 * A call to flush() is required before calling destroy.
	 */
	void (*destroy) (ike_sa_manager_t *this);
};

/**
 * Create the IKE_SA manager.
 *
 * @returns 	ike_sa_manager_t object, NULL if initialization fails
 */
ike_sa_manager_t *ike_sa_manager_create(void);

#endif /** IKE_SA_MANAGER_H_ @}*/

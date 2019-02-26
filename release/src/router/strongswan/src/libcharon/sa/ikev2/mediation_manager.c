/*
 * Copyright (C) 2007 Tobias Brunner
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

#include "mediation_manager.h"

#include <daemon.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>
#include <processing/jobs/mediation_job.h>

typedef struct peer_t peer_t;

/**
 * An entry in the linked list.
 */
struct peer_t {
	/** id of the peer */
	identification_t *id;

	/** sa id of the peer, NULL if offline */
	ike_sa_id_t *ike_sa_id;

	/** list of peer ids that reuested this peer */
	linked_list_t *requested_by;
};

/**
 * Implementation of peer_t.destroy.
 */
static void peer_destroy(peer_t *this)
{
	DESTROY_IF(this->id);
	DESTROY_IF(this->ike_sa_id);
	this->requested_by->destroy_offset(this->requested_by,
									   offsetof(identification_t, destroy));
	free(this);
}

/**
 * Creates a new entry for the list.
 */
static peer_t *peer_create(identification_t *id, ike_sa_id_t* ike_sa_id)
{
	peer_t *this;
	INIT(this,
		.id = id->clone(id),
		.ike_sa_id = ike_sa_id ? ike_sa_id->clone(ike_sa_id) : NULL,
		.requested_by = linked_list_create(),
	);
	return this;
}

typedef struct private_mediation_manager_t private_mediation_manager_t;

/**
 * Additional private members of mediation_manager_t.
 */
struct private_mediation_manager_t {
	/**
	 * Public interface of mediation_manager_t.
	 */
	 mediation_manager_t public;

	 /**
	  * Lock for exclusivly accessing the manager.
	  */
	 mutex_t *mutex;

	 /**
	  * Linked list with state entries.
	  */
	 linked_list_t *peers;
};

/**
 * Registers a peer's ID at another peer, if it is not yet registered
 */
static void register_peer(peer_t *peer, identification_t *peer_id)
{
	enumerator_t *enumerator;
	identification_t *current;

	enumerator = peer->requested_by->create_enumerator(peer->requested_by);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (peer_id->equals(peer_id, current))
		{
			enumerator->destroy(enumerator);
			return;
		}
	}
	enumerator->destroy(enumerator);

	peer->requested_by->insert_last(peer->requested_by,
									peer_id->clone(peer_id));
}

/**
 * Get a peer_t object by a peer's id
 */
static status_t get_peer_by_id(private_mediation_manager_t *this,
							   identification_t *id, peer_t **peer)
{
	enumerator_t *enumerator;
	peer_t *current;
	status_t status = NOT_FOUND;

	enumerator = this->peers->create_enumerator(this->peers);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (id->equals(id, current->id))
		{
			if (peer)
			{
				*peer = current;
			}
			status = SUCCESS;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return status;
}

/**
 * Check if a given peer is registered at other peers. If so, remove it there
 * and then remove peers completely that are not online and have no registered
 * peers.
 */
static void unregister_peer(private_mediation_manager_t *this,
							identification_t *peer_id)
{
	enumerator_t *enumerator, *enumerator_r;
	peer_t *peer;
	identification_t *registered;

	enumerator = this->peers->create_enumerator(this->peers);
	while (enumerator->enumerate(enumerator, (void**)&peer))
	{
		enumerator_r = peer->requested_by->create_enumerator(peer->requested_by);
		while (enumerator_r->enumerate(enumerator_r, (void**)&registered))
		{
			if (peer_id->equals(peer_id, registered))
			{
				peer->requested_by->remove_at(peer->requested_by, enumerator_r);
				registered->destroy(registered);
				break;
			}
		}
		enumerator_r->destroy(enumerator_r);

		if (!peer->ike_sa_id &&
			!peer->requested_by->get_count(peer->requested_by))
		{
			this->peers->remove_at(this->peers, enumerator);
			peer_destroy(peer);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(mediation_manager_t, remove_sa, void,
	private_mediation_manager_t *this, ike_sa_id_t *ike_sa_id)
{
	enumerator_t *enumerator;
	peer_t *peer;

	this->mutex->lock(this->mutex);

	enumerator = this->peers->create_enumerator(this->peers);
	while (enumerator->enumerate(enumerator, (void**)&peer))
	{
		if (ike_sa_id->equals(ike_sa_id, peer->ike_sa_id))
		{
			this->peers->remove_at(this->peers, enumerator);

			unregister_peer(this, peer->id);

			peer_destroy(peer);
			break;
		}
	}
	enumerator->destroy(enumerator);

	this->mutex->unlock(this->mutex);
}

METHOD(mediation_manager_t, update_sa_id, void,
	private_mediation_manager_t *this, identification_t *peer_id,
	ike_sa_id_t *ike_sa_id)
{
	enumerator_t *enumerator;
	peer_t *peer;
	bool found = FALSE;

	this->mutex->lock(this->mutex);

	enumerator = this->peers->create_enumerator(this->peers);
	while (enumerator->enumerate(enumerator, (void**)&peer))
	{
		if (peer_id->equals(peer_id, peer->id))
		{
			DESTROY_IF(peer->ike_sa_id);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		DBG2(DBG_IKE, "adding peer '%Y'", peer_id);
		peer = peer_create(peer_id, NULL);
		this->peers->insert_last(this->peers, peer);
	}

	DBG2(DBG_IKE, "changing registered IKE_SA ID of peer '%Y'", peer_id);
	peer->ike_sa_id = ike_sa_id ? ike_sa_id->clone(ike_sa_id) : NULL;

	/* send callbacks to registered peers */
	identification_t *requester;
	while(peer->requested_by->remove_last(peer->requested_by,
										  (void**)&requester) == SUCCESS)
	{
		job_t *job = (job_t*)mediation_callback_job_create(requester, peer_id);
		lib->processor->queue_job(lib->processor, job);
		requester->destroy(requester);
	}

	this->mutex->unlock(this->mutex);
}

METHOD(mediation_manager_t, check, ike_sa_id_t*,
	private_mediation_manager_t *this, identification_t *peer_id)
{
	peer_t *peer;
	ike_sa_id_t *ike_sa_id;

	this->mutex->lock(this->mutex);

	if (get_peer_by_id(this, peer_id, &peer) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		return NULL;
	}

	ike_sa_id = peer->ike_sa_id;

	this->mutex->unlock(this->mutex);

	return ike_sa_id;
}

METHOD(mediation_manager_t, check_and_register, ike_sa_id_t*,
	private_mediation_manager_t *this, identification_t *peer_id,
	identification_t *requester)
{
	peer_t *peer;
	ike_sa_id_t *ike_sa_id;

	this->mutex->lock(this->mutex);

	if (get_peer_by_id(this, peer_id, &peer) != SUCCESS)
	{
		DBG2(DBG_IKE, "adding peer %Y", peer_id);
		peer = peer_create(peer_id, NULL);
		this->peers->insert_last(this->peers, peer);
	}

	if (!peer->ike_sa_id)
	{
		/* the peer is not online */
		DBG2(DBG_IKE, "requested peer '%Y' is offline, registering peer '%Y'",
			 peer_id, requester);
		register_peer(peer, requester);
		this->mutex->unlock(this->mutex);
		return NULL;
	}

	ike_sa_id = peer->ike_sa_id;

	this->mutex->unlock(this->mutex);

	return ike_sa_id;
}

METHOD(mediation_manager_t, destroy, void,
	private_mediation_manager_t *this)
{
	this->mutex->lock(this->mutex);

	this->peers->destroy_function(this->peers, (void*)peer_destroy);

	this->mutex->unlock(this->mutex);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
mediation_manager_t *mediation_manager_create()
{
	private_mediation_manager_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
			.remove = _remove_sa,
			.update_sa_id = _update_sa_id,
			.check = _check,
			.check_and_register = _check_and_register,
		},
		.peers = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);
	return &this->public;
}

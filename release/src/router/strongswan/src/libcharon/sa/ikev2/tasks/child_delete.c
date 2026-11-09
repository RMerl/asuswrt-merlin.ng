/*
 * Copyright (C) 2009-2022 Tobias Brunner
 * Copyright (C) 2006-2007 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include "child_delete.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_child_sa_job.h>
#include <sa/ikev2/tasks/child_create.h>
#include <sa/ikev2/tasks/child_rekey.h>

#ifndef DELETE_REKEYED_DELAY
#define DELETE_REKEYED_DELAY 5
#endif

typedef struct private_child_delete_t private_child_delete_t;

/**
 * Private members of a child_delete_t task.
 */
struct private_child_delete_t {

	/**
	 * Public methods and task_t interface.
	 */
	child_delete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Whether we are the initiator of the exchange
	 */
	bool initiator;

	/**
	 * Protocol of CHILD_SA to delete (as initiator)
	 */
	protocol_id_t protocol;

	/**
	 * Inbound SPI of CHILD_SA to delete (as initiator)
	 */
	uint32_t spi;

	/**
	 * CHILD_SA already expired (as initiator)
	 */
	bool expired;

	/**
	 * CHILD_SAs which get deleted, entry_t*
	 */
	linked_list_t *child_sas;
};

/**
 * Information about a deleted CHILD_SA
 */
typedef struct {
	/** Deleted CHILD_SA */
	child_sa_t *child_sa;
	/** The original state of the CHILD_SA */
	child_sa_state_t orig_state;
	/** How this CHILD_SA collides with an active rekeying */
	child_rekey_collision_t collision;
} entry_t;

CALLBACK(match_child, bool,
	entry_t *entry, va_list args)
{
	child_sa_t *child_sa;

	VA_ARGS_VGET(args, child_sa);
	return entry->child_sa == child_sa;
}

/**
 * build the delete payloads from the listed child_sas
 */
static void build_payloads(private_child_delete_t *this, message_t *message)
{
	delete_payload_t *ah = NULL, *esp = NULL;
	enumerator_t *enumerator;
	entry_t *entry;
	protocol_id_t protocol;
	uint32_t spi;

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		protocol = entry->child_sa->get_protocol(entry->child_sa);
		spi = entry->child_sa->get_spi(entry->child_sa, TRUE);

		switch (protocol)
		{
			case PROTO_ESP:
				if (!esp)
				{
					esp = delete_payload_create(PLV2_DELETE, PROTO_ESP);
					message->add_payload(message, (payload_t*)esp);
				}
				esp->add_spi(esp, spi);
				DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));
				break;
			case PROTO_AH:
				if (ah == NULL)
				{
					ah = delete_payload_create(PLV2_DELETE, PROTO_AH);
					message->add_payload(message, (payload_t*)ah);
				}
				ah->add_spi(ah, spi);
				DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Install the outbound SA of the CHILD_SA that replaced the given CHILD_SA
 * in a rekeying.
 */
static void conclude_rekeying(private_child_delete_t *this, child_sa_t *old)
{
	child_sa_t *child_sa;

	child_sa = old->get_rekey_sa(old);
	old->set_rekey_sa(old, NULL);
	child_sa->set_rekey_sa(child_sa, NULL);
	child_rekey_conclude_rekeying(old, child_sa);
}

/**
 * Queue a task to recreate the given CHILD_SA.
 */
static void queue_child_create(ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	child_create_t *child_create;
	child_cfg_t *child_cfg;
	uint32_t reqid;

	child_cfg = child_sa->get_config(child_sa);
	child_create = child_create_create(ike_sa, child_cfg->get_ref(child_cfg),
									   FALSE, NULL, NULL, 0);
	child_create->recreate_sa(child_create, child_sa);
	reqid = child_sa->get_reqid_ref(child_sa);
	if (reqid)
	{
		child_create->use_reqid(child_create, reqid);
		charon->kernel->release_reqid(charon->kernel, reqid);
	}
	child_create->use_label(child_create, child_sa->get_label(child_sa));
	child_create->use_per_cpu(child_create, child_sa->use_per_cpu(child_sa),
							  child_sa->get_cpu(child_sa));
	ike_sa->queue_task(ike_sa, (task_t*)child_create);
}

/**
 * Destroy and optionally reestablish the given CHILD_SA according to config.
 */
static status_t destroy_and_reestablish_internal(ike_sa_t *ike_sa,
												 child_sa_t *child_sa,
												 bool trigger_updown,
												 bool delete_action,
												 action_t forced_action)
{
	child_cfg_t *child_cfg;
	action_t action;
	bool initiate = FALSE;

	child_sa->set_state(child_sa, CHILD_DELETED);
	if (trigger_updown)
	{
		charon->bus->child_updown(charon->bus, child_sa, FALSE);
	}

	DBG1(DBG_IKE, "CHILD_SA %s{%u} closed", child_sa->get_name(child_sa),
		 child_sa->get_unique_id(child_sa));

	action = forced_action ?: child_sa->get_close_action(child_sa);

	if (delete_action)
	{
		if (action & ACTION_TRAP)
		{
			child_cfg = child_sa->get_config(child_sa);
			charon->traps->install(charon->traps,
								   ike_sa->get_peer_cfg(ike_sa),
								   child_cfg->get_ref(child_cfg));
		}
		if (action & ACTION_START)
		{
			queue_child_create(ike_sa, child_sa);
			initiate = TRUE;
		}
	}
	ike_sa->destroy_child_sa(ike_sa, child_sa->get_protocol(child_sa),
							 child_sa->get_spi(child_sa, TRUE));
	return initiate ? ike_sa->initiate(ike_sa, NULL, NULL) : SUCCESS;
}

/*
 * Described in header
 */
status_t child_delete_destroy_and_reestablish(ike_sa_t *ike_sa,
											  child_sa_t *child_sa)
{
	return destroy_and_reestablish_internal(ike_sa, child_sa, TRUE, TRUE, 0);
}

/*
 * Described in header
 */
status_t child_delete_destroy_and_force_reestablish(ike_sa_t *ike_sa,
													child_sa_t *child_sa)
{
	return destroy_and_reestablish_internal(ike_sa, child_sa, TRUE, TRUE,
											ACTION_START);
}

/*
 * Described in header
 */
void child_delete_destroy_rekeyed(ike_sa_t *ike_sa, child_sa_t *child_sa)
{
	time_t now, expire;
	u_int delay;

	/* make sure the SA is in the correct state and the outbound SA is not
	 * installed */
	child_sa->remove_outbound(child_sa);
	child_sa->set_state(child_sa, CHILD_DELETED);

	now = time_monotonic(NULL);
	delay = lib->settings->get_int(lib->settings, "%s.delete_rekeyed_delay",
								   DELETE_REKEYED_DELAY, lib->ns);

	expire = child_sa->get_lifetime(child_sa, TRUE);
	if (delay && (!expire || ((now + delay) < expire)))
	{
		DBG1(DBG_IKE, "delay closing of inbound CHILD_SA %s{%u} for %us",
			 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
			 delay);
		lib->scheduler->schedule_job(lib->scheduler,
			(job_t*)delete_child_sa_job_create_id(
							child_sa->get_unique_id(child_sa)), delay);
		return;
	}
	else if (now < expire)
	{
		/* let it expire naturally */
		DBG1(DBG_IKE, "let rekeyed inbound CHILD_SA %s{%u} expire naturally "
			 "in %us", child_sa->get_name(child_sa),
			 child_sa->get_unique_id(child_sa), expire-now);
		return;
	}
	/* no delay and no lifetime, destroy it immediately.  since we suppress
	 * actions, there is no need to check the return value */
	destroy_and_reestablish_internal(ike_sa, child_sa, FALSE, FALSE, 0);
}

/**
 * Check if the SA should be ignored and kept until a concurrent active rekeying
 * is concluded (the rekey task is responsible for destroying the CHILD_SA).
 */
static bool keep_while_rekeying(entry_t *entry)
{
	switch (entry->collision)
	{
		case CHILD_REKEY_COLLISION_NONE:
			break;
		case CHILD_REKEY_COLLISION_OLD:
			/* if the peer deletes the SA we are trying to rekey and there
			 * hasn't been a collision, it might have sent the delete before our
			 * request arrived.  but it could also be an incorrect delete sent
			 * after it processed our rekey request, which we'd have to ignore.
			 * the active rekey task will decide once it has the response */
			if (entry->orig_state == CHILD_REKEYING)
			{
				return TRUE;
			}
			/* if there was a collision, the peer is expected to delete the old
			 * SA only if it won the collision, the SA is in state CHILD_REKEYED
			 * in this case.  we don't completely ignore the SA and conclude the
			 * rekeying for it now to switch to the new outbound SA (the peer
			 * will remove the old inbound SA once it receives the DELETE
			 * response), but don't destroy the old SA yet even though we return
			 * FALSE here.
			 * the active rekey task will later decide if the delete was
			 * legitimate or an incorrect delete for the old SA */
			break;
		case CHILD_REKEY_COLLISION_PEER:
			/* the peer deletes the SA it created itself before we received
			 * the rekey response, this is either the redundant SA, which
			 * would be fine, or the winning SA it already is deleting for
			 * some reason (presumably, after also sending a delete for the
			 * rekeyed SA). let the active rekey task decide once it receives
			 * the response and knows who won the collision */
			return TRUE;
	}
	return FALSE;
}

/**
 * Log an SA we are not yet closing completely.
 */
static void log_kept_sa(entry_t *entry)
{
	DBG1(DBG_IKE, "keeping %s CHILD_SA %s{%u} until active rekeying is "
		 "concluded",
		 entry->collision == CHILD_REKEY_COLLISION_OLD ? "rekeyed"
													   : "peer's",
		 entry->child_sa->get_name(entry->child_sa),
		 entry->child_sa->get_unique_id(entry->child_sa));
}

/**
 * Destroy the children listed in this->child_sas, reestablish by policy
 */
static status_t destroy_and_reestablish(private_child_delete_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	child_sa_t *child_sa, *other;
	status_t status = SUCCESS;

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		child_sa = entry->child_sa;
		other = child_sa->get_rekey_sa(child_sa);

		/* check if we have to keep the SA during a collision with an active
		 * rekey task */
		if (keep_while_rekeying(entry))
		{
			/* if the peer deleted its own SA, reset the link to the old SA,
			 * which might already be reset if the peer deleted the old SA
			 * first (the active rekey task will eventually destroy both) */
			if (other && entry->collision == CHILD_REKEY_COLLISION_PEER)
			{
				child_sa->set_rekey_sa(child_sa, NULL);
				other->set_rekey_sa(other, NULL);

				/* reset the state of the old SA until the active rekey task is
				 * done, but only if it's not also getting deleted by the peer
				 * and is already in state DELETING. note that we won't end up
				 * here if the peer deleted the old SA first as the link between
				 * the two SAs would already be reset then. so this is only the
				 * case if the peer sends the deletes for both SAs in the same
				 * message and the payload for the old one comes after the one
				 * for its own SA */
				if (other->get_state(other) == CHILD_REKEYED)
				{
					other->set_state(other, CHILD_REKEYING);
				}
			}
			log_kept_sa(entry);
			continue;
		}

		child_sa->set_state(child_sa, CHILD_DELETED);

		if (entry->orig_state == CHILD_REKEYED)
		{
			/* conclude the rekeying as responder/loser. the initiator/winner
			 * already did this right after the rekeying was completed (or
			 * before a delete was initiated), but in some cases the outbound
			 * SA was not yet removed, make sure it is */
			if (other)
			{
				conclude_rekeying(this, child_sa);
			}
			else
			{
				child_sa->remove_outbound(child_sa);
			}

			/* if this is a delete for the SA we are actively rekeying, let the
			 * rekey task handle the SA appropriately once the collision is
			 * resolved.  otherwise, destroy the SA now, but usually delayed to
			 * process delayed packets */
			if (entry->collision == CHILD_REKEY_COLLISION_OLD)
			{
				log_kept_sa(entry);
			}
			else
			{
				child_delete_destroy_rekeyed(this->ike_sa, child_sa);
			}
		}
		else
		{
			/* regular CHILD_SA delete, with one special case after a lost
			 * collision.  usually, the peer will delete the old SA and we
			 * conclude the rekeying above.  however, if it deletes its winning
			 * SA first, we assume it wants to delete the CHILD_SA and we
			 * conclude the rekeying here to trigger the events correctly */
			if (other && entry->orig_state == CHILD_INSTALLED)
			{
				conclude_rekeying(this, other);
			}
			status = destroy_and_reestablish_internal(this->ike_sa, child_sa,
										TRUE, !this->initiator &&
										entry->orig_state == CHILD_INSTALLED, 0);
			if (status != SUCCESS)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	return status;
}

/**
 * Print a log message for every closed CHILD_SA
 */
static void log_children(private_child_delete_t *this)
{
	linked_list_t *my_ts, *other_ts;
	enumerator_t *enumerator;
	entry_t *entry;
	child_sa_t *child_sa;
	uint64_t bytes_in, bytes_out;

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		child_sa = entry->child_sa;
		my_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, TRUE));
		other_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));
		if (this->expired)
		{
			DBG0(DBG_IKE, "closing expired CHILD_SA %s{%u} "
				 "with SPIs %.8x_i %.8x_o and TS %#R === %#R",
				 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
				 ntohl(child_sa->get_spi(child_sa, TRUE)),
				 ntohl(child_sa->get_spi(child_sa, FALSE)), my_ts, other_ts);
		}
		else
		{
			child_sa->get_usestats(child_sa, TRUE, NULL, &bytes_in, NULL);
			child_sa->get_usestats(child_sa, FALSE, NULL, &bytes_out, NULL);

			DBG0(DBG_IKE, "closing CHILD_SA %s{%u} with SPIs %.8x_i "
				 "(%llu bytes) %.8x_o (%llu bytes) and TS %#R === %#R",
				 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
				 ntohl(child_sa->get_spi(child_sa, TRUE)), bytes_in,
				 ntohl(child_sa->get_spi(child_sa, FALSE)), bytes_out,
				 my_ts, other_ts);
		}
		my_ts->destroy(my_ts);
		other_ts->destroy(other_ts);
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_child_delete_t *this, message_t *message)
{
	child_sa_t *child_sa, *other;
	entry_t *entry;

	child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
										  this->spi, TRUE);
	if (!child_sa)
	{
		/* check if it is an outbound SA */
		child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
											  this->spi, FALSE);
		if (!child_sa)
		{
			/* child does not exist anymore, abort exchange */
			message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
			return SUCCESS;
		}
		/* we work only with the inbound SPI */
		this->spi = child_sa->get_spi(child_sa, TRUE);
	}

	/* check if this SA is involved in a passive rekeying, either the old
	 * rekeyed one or the new one created by the peer */
	other = child_sa->get_rekey_sa(child_sa);
	if (other)
	{
		if (child_sa->get_state(child_sa) == CHILD_REKEYED)
		{
			/* the peer was expected to delete this rekeyed SA.  we don't send a
			 * DELETE, in particular, if this is triggered by an expire, because
			 * that could cause a collision if the CREATE_CHILD_SA response is
			 * delayed (the peer might interpret that as a deletion of the SA by
			 * a user and might then ignore the CREATE_CHILD_SA response once it
			 * arrives - like old strongSwan versions did - although it
			 * shouldn't as we properly replied to that request so only a delete
			 * for the new CHILD_SA should result in a deletion) */
			child_sa->set_state(child_sa, CHILD_DELETED);
			conclude_rekeying(this, child_sa);
		}
		else
		{
			/* the rekeying for the new SA we are about to delete on the user's
			 * behalf has not yet been completed, that is, we are waiting for
			 * the delete for the old SA and have not yet fully installed this
			 * new one.  we do that now so events are triggered properly when
			 * we delete it */
			DBG2(DBG_IKE, "complete rekeying for %s{%u} before deleting "
				 "replacement CHILD_SA %s{%u}",
				 other->get_name(other), other->get_unique_id(other),
				 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa));
			conclude_rekeying(this, other);
		}
	}

	if (child_sa->get_state(child_sa) == CHILD_DELETED)
	{
		/* DELETEs for this CHILD_SA were already exchanged, but it was not yet
		 * destroyed to allow delayed packets to get processed, or we suppress
		 * the DELETE explicitly (see above) */
		destroy_and_reestablish_internal(this->ike_sa, child_sa, FALSE, FALSE, 0);
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		return SUCCESS;
	}

	INIT(entry,
		.child_sa = child_sa,
		.orig_state = child_sa->get_state(child_sa),
	);
	child_sa->set_state(child_sa, CHILD_DELETING);
	this->child_sas->insert_last(this->child_sas, entry);

	log_children(this);
	build_payloads(this, message);

	if (this->expired)
	{
		DBG1(DBG_IKE, "queue CHILD_SA recreate after hard expire");
		queue_child_create(this->ike_sa, child_sa);
	}
	return NEED_MORE;
}

/**
 * Check if the given CHILD_SA is the SA created by the peer in a rekey
 * collision and allow the active rekey task to collect the SPI if it's not yet
 * known, in which case it could be for the SA we created in an active rekeying
 * that we haven't yet completed.
 */
static child_rekey_collision_t possible_rekey_collision(
												private_child_delete_t *this,
												child_sa_t *child, uint32_t spi)
{
	enumerator_t *tasks;
	task_t *task;
	child_rekey_t *rekey;
	child_rekey_collision_t collision = CHILD_REKEY_COLLISION_NONE;

	tasks = this->ike_sa->create_task_enumerator(this->ike_sa,
												 TASK_QUEUE_ACTIVE);
	while (tasks->enumerate(tasks, &task))
	{
		if (task->get_type(task) == TASK_CHILD_REKEY)
		{
			rekey = (child_rekey_t*)task;
			collision = rekey->handle_delete(rekey, child, spi);
			break;
		}
	}
	tasks->destroy(tasks);
	return collision;
}

/**
 * Read payloads and find the children to delete.
 */
static void process_payloads(private_child_delete_t *this, message_t *message)
{
	enumerator_t *payloads, *spis;
	payload_t *payload;
	delete_payload_t *delete_payload;
	uint32_t spi;
	protocol_id_t protocol;
	child_sa_t *child_sa;
	entry_t *entry;

	payloads = message->create_payload_enumerator(message);
	while (payloads->enumerate(payloads, &payload))
	{
		if (payload->get_type(payload) == PLV2_DELETE)
		{
			delete_payload = (delete_payload_t*)payload;
			protocol = delete_payload->get_protocol_id(delete_payload);
			if (protocol != PROTO_ESP && protocol != PROTO_AH)
			{
				continue;
			}
			spis = delete_payload->create_spi_enumerator(delete_payload);
			while (spis->enumerate(spis, &spi))
			{
				child_rekey_collision_t collision = CHILD_REKEY_COLLISION_NONE;

				child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol,
													  spi, FALSE);
				if (!this->initiator)
				{
					collision = possible_rekey_collision(this, child_sa, spi);
				}
				if (!child_sa)
				{
					DBG1(DBG_IKE, "received DELETE for unknown %N CHILD_SA with"
						 " SPI %.8x", protocol_id_names, protocol, ntohl(spi));
					continue;
				}
				DBG1(DBG_IKE, "received DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));

				if (this->child_sas->find_first(this->child_sas, match_child,
												NULL, child_sa))
				{
					continue;
				}
				else if (this->initiator)
				{
					DBG1(DBG_IKE, "ignore DELETE for %N CHILD_SA with SPI "
						 "%.8x in response, didn't request its deletion",
						 protocol_id_names, protocol, ntohl(spi));
					continue;
				}

				INIT(entry,
					.child_sa = child_sa,
					.orig_state = child_sa->get_state(child_sa),
					.collision = collision,
				);
				if (entry->orig_state == CHILD_DELETED ||
					entry->orig_state == CHILD_DELETING)
				{
					/* we either already deleted but have not yet destroyed the
					 * SA, which we ignore; or we're actively deleting it, in
					 * which case we don't send back a DELETE either */
					free(entry);
					continue;
				}
				child_sa->set_state(child_sa, CHILD_DELETING);
				this->child_sas->insert_last(this->child_sas, entry);
			}
			spis->destroy(spis);
		}
	}
	payloads->destroy(payloads);
}

METHOD(task_t, process_i, status_t,
	private_child_delete_t *this, message_t *message)
{
	process_payloads(this, message);
	return destroy_and_reestablish(this);
}

METHOD(task_t, process_r, status_t,
	private_child_delete_t *this, message_t *message)
{
	process_payloads(this, message);
	log_children(this);
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_child_delete_t *this, message_t *message)
{
	build_payloads(this, message);
	return destroy_and_reestablish(this);
}

METHOD(task_t, get_type, task_type_t,
	private_child_delete_t *this)
{
	return TASK_CHILD_DELETE;
}

METHOD(task_t, migrate, void,
	private_child_delete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;

	this->child_sas->destroy_function(this->child_sas, free);
	this->child_sas = linked_list_create();
}

METHOD(task_t, destroy, void,
	private_child_delete_t *this)
{
	this->child_sas->destroy_function(this->child_sas, free);
	free(this);
}

/*
 * Described in header.
 */
child_delete_t *child_delete_create(ike_sa_t *ike_sa, protocol_id_t protocol,
									uint32_t spi, bool expired)
{
	private_child_delete_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.child_sas = linked_list_create(),
		.protocol = protocol,
		.spi = spi,
		.expired = expired,
	);

	if (protocol != PROTO_NONE)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
		this->initiator = TRUE;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
		this->initiator = FALSE;
	}
	return &this->public;
}

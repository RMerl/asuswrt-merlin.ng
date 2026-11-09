/*
 * Copyright (C) 2009-2023 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "child_rekey.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <sa/ikev2/tasks/child_create.h>
#include <sa/ikev2/tasks/child_delete.h>
#include <processing/jobs/rekey_child_sa_job.h>
#include <processing/jobs/rekey_ike_sa_job.h>


typedef struct private_child_rekey_t private_child_rekey_t;

/**
 * Private members of a child_rekey_t task.
 */
struct private_child_rekey_t {

	/**
	 * Public methods and task_t interface.
	 */
	child_rekey_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Protocol of CHILD_SA to rekey
	 */
	protocol_id_t protocol;

	/**
	 * Inbound SPI of CHILD_SA to rekey
	 */
	uint32_t spi;

	/**
	 * Encoded SPI in REKEY_SA notify if no CHILD_SA is found
	 */
	chunk_t spi_data;

	/**
	 * the CHILD_CREATE task which is reused to simplify rekeying
	 */
	child_create_t *child_create;

	/**
	 * the CHILD_DELETE task to delete rekeyed CHILD_SA
	 */
	child_delete_t *child_delete;

	/**
	 * CHILD_SA which gets rekeyed
	 */
	child_sa_t *child_sa;

	/**
	 * Colliding passive rekey task
	 */
	task_t *collision;

	/**
	 * SPIs of SAs the peer deleted and we haven't found while this task was
	 * active
	 */
	array_t *deleted_spis;

	/**
	 * State flags
	 */
	enum {

		/**
		 * Set if we use multiple key exchanges and already processed the
		 * CREATE_CHILD_SA response and started sending IKE_FOLLOWUP_KEs.
		 */
		CHILD_REKEY_FOLLOWUP_KE = (1<<0),

		/**
		 * Set if the passive rekey task is completed and we adopted it,
		 * otherwise (i.e. for  multi-KE rekeyings) we just reference it.
		 */
		CHILD_REKEY_PASSIVE_INSTALLED = (1<<1),

		/**
		 * Indicates that the peer sent a DELETE for its own CHILD_SA of a
		 * collision.  In regular rekeyings, this happens if a peer lost and
		 * the delete for the redundant SA gets processed before the active
		 * rekey job is complete.  It could also mean the peer deleted its
		 * winning SA.
		 */
		CHILD_REKEY_OTHER_DELETED = (1<<2),

		/**
		 * Indicates that the peer sent a DELETE for the rekeyed/old CHILD_SA.
		 * This happens if the peer has won the rekey collision, but it might
		 * also happen if it incorrectly sent one after it replied to our
		 * CREATE_CHILD_SA request and the DELETE arrived before that response.
		 */
		CHILD_REKEY_OLD_SA_DELETED = (1<<3),

		/**
		 * After handling the collision, this indicates whether the peer deleted
		 * the winning replacement SA (either ours or its own).
		 */
		CHILD_REKEY_REPLACEMENT_DELETED = (1<<4),

	} flags;
};

/**
 * Schedule a retry if rekeying temporary failed
 */
static void schedule_delayed_rekey(private_child_rekey_t *this)
{
	uint32_t retry;
	job_t *job;

	retry = RETRY_INTERVAL - (random() % RETRY_JITTER);
	job = (job_t*)rekey_child_sa_job_create(
						this->child_sa->get_protocol(this->child_sa),
						this->child_sa->get_spi(this->child_sa, TRUE),
						this->ike_sa->get_my_host(this->ike_sa));
	DBG1(DBG_IKE, "CHILD_SA rekeying failed, trying again in %d seconds", retry);
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
	lib->scheduler->schedule_job(lib->scheduler, job, retry);
}

METHOD(task_t, build_i_delete, status_t,
	private_child_rekey_t *this, message_t *message)
{
	/* update exchange type to INFORMATIONAL for the delete */
	message->set_exchange_type(message, INFORMATIONAL);
	return this->child_delete->task.build(&this->child_delete->task, message);
}

METHOD(task_t, process_i_delete, status_t,
	private_child_rekey_t *this, message_t *message)
{
	return this->child_delete->task.process(&this->child_delete->task, message);
}

/**
 * In failure cases, we don't use a child_delete task, but handle the deletes
 * ourselves for more flexibility (in particular, adding multiple DELETE
 * payloads to a single message).
 */
static void build_delete_old_sa(private_child_rekey_t *this, message_t *message)
{
	delete_payload_t *del;
	protocol_id_t protocol;
	uint32_t spi;

	message->set_exchange_type(message, INFORMATIONAL);

	protocol = this->child_sa->get_protocol(this->child_sa);
	spi = this->child_sa->get_spi(this->child_sa, TRUE);

	del = delete_payload_create(PLV2_DELETE, protocol);
	del->add_spi(del, spi);
	message->add_payload(message, (payload_t*)del);

	DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
		 protocol_id_names, protocol, ntohl(spi));
}

METHOD(task_t, build_i_delete_replacement, status_t,
	private_child_rekey_t *this, message_t *message)
{
	/* add the delete for the replacement we failed to create locally but the
	 * peer probably already has installed */
	this->child_create->task.build(&this->child_create->task, message);
	return SUCCESS;
}

METHOD(task_t, build_i_delete_old_destroy, status_t,
	private_child_rekey_t *this, message_t *message)
{
	/* send the delete but then immediately destroy and possibly recreate the
	 * CHILD_SA as the peer deleted its replacement.  treat this like the peer
	 * sent a delete for the original SA */
	build_delete_old_sa(this, message);
	child_delete_destroy_and_reestablish(this->ike_sa, this->child_sa);
	return SUCCESS;
}

/**
 * Delete either both or only the replacement SA and then destroy and recreate
 * the old SA.
 */
static status_t build_delete_recreate(private_child_rekey_t *this,
									  message_t *message, bool delete_old)
{
	if (delete_old)
	{
		build_delete_old_sa(this, message);
	}
	this->child_create->task.build(&this->child_create->task, message);
	child_delete_destroy_and_force_reestablish(this->ike_sa, this->child_sa);
	return SUCCESS;
}

METHOD(task_t, build_i_delete_replacement_recreate, status_t,
	private_child_rekey_t *this, message_t *message)
{
	return build_delete_recreate(this, message, FALSE);
}

METHOD(task_t, build_i_delete_both_recreate, status_t,
	private_child_rekey_t *this, message_t *message)
{
	return build_delete_recreate(this, message, TRUE);
}

METHOD(task_t, build_i, status_t,
	private_child_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;

	this->child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
												this->spi, TRUE);
	if (!this->child_sa)
	{	/* check if it is an outbound CHILD_SA */
		this->child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
													this->spi, FALSE);
		if (this->child_sa)
		{
			/* we work only with the inbound SPI */
			this->spi = this->child_sa->get_spi(this->child_sa, TRUE);
		}
	}
	if (!this->child_sa ||
		(!this->child_create &&
		  this->child_sa->get_state(this->child_sa) != CHILD_INSTALLED) ||
		(this->child_create &&
		 this->child_sa->get_state(this->child_sa) != CHILD_REKEYING))
	{
		/* CHILD_SA is gone or in the wrong state, unable to rekey */
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		return SUCCESS;
	}

	/* our CHILD_CREATE task does the hard work for us */
	if (!this->child_create)
	{
		child_cfg_t *config;
		uint32_t reqid;

		config = this->child_sa->get_config(this->child_sa);
		this->child_create = child_create_create(this->ike_sa,
								config->get_ref(config), TRUE, NULL, NULL, 0);

		this->child_create->recreate_sa(this->child_create, this->child_sa);

		reqid = this->child_sa->get_reqid_ref(this->child_sa);
		if (reqid)
		{
			this->child_create->use_reqid(this->child_create, reqid);
			charon->kernel->release_reqid(charon->kernel, reqid);
		}
		this->child_create->use_marks(this->child_create,
						this->child_sa->get_mark(this->child_sa, TRUE).value,
						this->child_sa->get_mark(this->child_sa, FALSE).value);
		this->child_create->use_if_ids(this->child_create,
						this->child_sa->get_if_id(this->child_sa, TRUE),
						this->child_sa->get_if_id(this->child_sa, FALSE));
		this->child_create->use_label(this->child_create,
						this->child_sa->get_label(this->child_sa));
		this->child_create->use_per_cpu(this->child_create,
						this->child_sa->use_per_cpu(this->child_sa),
						this->child_sa->get_cpu(this->child_sa));
	}

	if (this->child_create->task.build(&this->child_create->task,
									   message) != NEED_MORE)
	{
		schedule_delayed_rekey(this);
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		return SUCCESS;
	}
	if (message->get_exchange_type(message) == CREATE_CHILD_SA)
	{
		/* don't add the notify if the CHILD_CREATE task changed the exchange */
		notify = notify_payload_create_from_protocol_and_type(PLV2_NOTIFY,
													this->protocol, REKEY_SA);
		notify->set_spi(notify, this->spi);
		message->add_payload(message, (payload_t*)notify);
	}
	this->child_sa->set_state(this->child_sa, CHILD_REKEYING);

	return NEED_MORE;
}

/**
 * Find a CHILD_SA using the REKEY_SA notify
 */
static void find_child(private_child_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;
	protocol_id_t protocol;
	uint32_t spi;
	child_sa_t *child_sa;

	notify = message->get_notify(message, REKEY_SA);
	if (notify)
	{
		protocol = notify->get_protocol_id(notify);
		spi = notify->get_spi(notify);

		if (protocol == PROTO_ESP || protocol == PROTO_AH)
		{
			child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol,
												  spi, FALSE);
			/* ignore rekeyed/deleted CHILD_SAs we keep around */
			if (child_sa &&
				child_sa->get_state(child_sa) != CHILD_DELETED)
			{
				this->child_sa = child_sa;
			}
		}
		if (!this->child_sa)
		{
			this->protocol = protocol;
			this->spi_data = chunk_clone(notify->get_spi_data(notify));
		}
	}
}

METHOD(task_t, process_r, status_t,
	private_child_rekey_t *this, message_t *message)
{
	/* let the CHILD_CREATE task process the message */
	this->child_create->task.process(&this->child_create->task, message);

	find_child(this, message);

	return NEED_MORE;
}

/**
 * Check if we are actively rekeying and, optionally, if we already sent an
 * IKE_FOLLOWUP_KE message.
 */
static bool actively_rekeying(private_child_rekey_t *this, bool *followup_sent)
{
	enumerator_t *enumerator;
	task_t *task;
	bool found = FALSE;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_ACTIVE);
	while (enumerator->enumerate(enumerator, (void**)&task))
	{
		if (task->get_type(task) == TASK_CHILD_REKEY)
		{
			private_child_rekey_t *rekey = (private_child_rekey_t*)task;

			if (this->child_sa == rekey->child_sa)
			{
				if (followup_sent)
				{
					*followup_sent = rekey->flags & CHILD_REKEY_FOLLOWUP_KE;
				}
				found = TRUE;
			}
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(task_t, build_r, status_t,
	private_child_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;
	child_cfg_t *config;
	child_sa_t *child_sa, *old_replacement;
	child_sa_state_t state = CHILD_INSTALLED;
	uint32_t reqid;
	bool followup_sent = FALSE;

	if (!this->child_sa)
	{
		DBG1(DBG_IKE, "unable to rekey, %N CHILD_SA with SPI %+B not found",
			 protocol_id_names, this->protocol, &this->spi_data);
		notify = notify_payload_create_from_protocol_and_type(PLV2_NOTIFY,
											this->protocol, CHILD_SA_NOT_FOUND);
		notify->set_spi_data(notify, this->spi_data);
		message->add_payload(message, (payload_t*)notify);
		return SUCCESS;
	}
	if (this->child_sa->get_state(this->child_sa) == CHILD_DELETING)
	{
		DBG1(DBG_IKE, "unable to rekey CHILD_SA %s{%u}, we are deleting it",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}
	if (actively_rekeying(this, &followup_sent) && followup_sent)
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but we did too and already "
			 "sent IKE_FOLLOWUP_KE");
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}

	if (message->get_exchange_type(message) == CREATE_CHILD_SA)
	{
		this->child_create->recreate_sa(this->child_create, this->child_sa);
		reqid = this->child_sa->get_reqid_ref(this->child_sa);
		if (reqid)
		{
			this->child_create->use_reqid(this->child_create, reqid);
			charon->kernel->release_reqid(charon->kernel, reqid);
		}
		this->child_create->use_marks(this->child_create,
						this->child_sa->get_mark(this->child_sa, TRUE).value,
						this->child_sa->get_mark(this->child_sa, FALSE).value);
		this->child_create->use_if_ids(this->child_create,
						this->child_sa->get_if_id(this->child_sa, TRUE),
						this->child_sa->get_if_id(this->child_sa, FALSE));
		this->child_create->use_label(this->child_create,
						this->child_sa->get_label(this->child_sa));
		this->child_create->use_per_cpu(this->child_create,
						this->child_sa->use_per_cpu(this->child_sa),
						this->child_sa->get_cpu(this->child_sa));
		config = this->child_sa->get_config(this->child_sa);
		this->child_create->set_config(this->child_create,
									   config->get_ref(config));
		state = this->child_sa->get_state(this->child_sa);
		this->child_sa->set_state(this->child_sa, CHILD_REKEYING);
	}

	if (this->child_create->task.build(&this->child_create->task,
									   message) == NEED_MORE)
	{
		/* additional key exchanges */
		this->flags |= CHILD_REKEY_FOLLOWUP_KE;
		return NEED_MORE;
	}

	child_sa = this->child_create->get_child(this->child_create);
	if (child_sa && child_sa->get_state(child_sa) == CHILD_INSTALLED)
	{
		this->child_sa->set_state(this->child_sa, CHILD_REKEYED);
		/* we've seen peers sending multiple rekey requests (probably a bug in
		 * their collision handling), so make sure we unlink any previous SA */
		old_replacement = this->child_sa->get_rekey_sa(this->child_sa);
		if (old_replacement)
		{
			old_replacement->set_rekey_sa(old_replacement, NULL);
		}
		/* link the SAs to handle possible collisions */
		this->child_sa->set_rekey_sa(this->child_sa, child_sa);
		child_sa->set_rekey_sa(child_sa, this->child_sa);
		/* like installing the outbound SA, we only trigger the child-rekey
		 * event once the old SA is deleted */
	}
	else if (this->child_sa->get_state(this->child_sa) == CHILD_REKEYING)
	{	/* rekeying failed, reuse old child */
		this->child_sa->set_state(this->child_sa, state);
	}
	return SUCCESS;
}

/**
 * Check if the peer deleted the replacement SA we created while we waited for
 * its completion.
 */
static bool is_our_replacement_deleted(private_child_rekey_t *this)
{
	uint32_t spi, peer_spi;
	int i;

	if (!this->deleted_spis)
	{
		return FALSE;
	}

	peer_spi = this->child_create->get_other_spi(this->child_create);
	if (!peer_spi)
	{
		return FALSE;
	}

	for (i = 0; i < array_count(this->deleted_spis); i++)
	{
		array_get(this->deleted_spis, i, &spi);
		if (spi == peer_spi)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Remove the passive rekey task that's waiting for IKE_FOLLOWUP_KE requests
 * that will never come if we won the collision.
 */
static void remove_passive_rekey_task(private_child_rekey_t *this)
{
	enumerator_t *enumerator;
	task_t *task;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_PASSIVE);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_CHILD_REKEY)
		{
			this->ike_sa->remove_task(this->ike_sa, enumerator);
			task->destroy(task);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Compare the nonces to determine if we lost the rekey collision.
 * The SA with the lowest nonce should be deleted (if already complete), this
 * checks if we or the peer created it
 */
static bool lost_collision(private_child_rekey_t *this)
{
	private_child_rekey_t *other = (private_child_rekey_t*)this->collision;
	chunk_t this_nonce, other_nonce;

	if (!other)
	{
		return FALSE;
	}

	this_nonce = this->child_create->get_lower_nonce(this->child_create);
	other_nonce = other->child_create->get_lower_nonce(other->child_create);

	return memcmp(this_nonce.ptr, other_nonce.ptr,
				  min(this_nonce.len, other_nonce.len)) < 0;
}

/**
 * Handle a rekey collision.  Returns TRUE if we won the collision or there
 * wasn't one.  Also returns the SA that should be deleted and the winning SA
 * of the collision, if any.
 */
static bool handle_collision(private_child_rekey_t *this,
							 child_sa_t **to_delete, child_sa_t **winning_sa,
							 bool multi_ke)
{
	private_child_rekey_t *other = (private_child_rekey_t*)this->collision;
	child_sa_t *other_sa;

	if (lost_collision(this))
	{
		*to_delete = this->child_create->get_child(this->child_create);
		if (multi_ke)
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision lost, abort incomplete "
				 "multi-KE rekeying");
		}
		else
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision lost, deleting "
				 "redundant child %s{%u}", (*to_delete)->get_name(*to_delete),
				 (*to_delete)->get_unique_id(*to_delete));
		}
		/* check if the passive rekeying is completed */
		if (this->flags & CHILD_REKEY_PASSIVE_INSTALLED)
		{
			*winning_sa = other->child_create->get_child(other->child_create);

			if (this->flags & CHILD_REKEY_OTHER_DELETED)
			{
				/* the peer deleted its own replacement SA while we waited
				 * for a response, set a flag to destroy the SA accordingly */
				this->flags |= CHILD_REKEY_REPLACEMENT_DELETED;
				/* if the peer has not triggered a rekey event yet by deleting
				 * its own SA before deleting the old SA (if it did so at all),
				 * we trigger that now so listeners can track this properly */
				if (!(this->flags & CHILD_REKEY_OLD_SA_DELETED) ||
					(*winning_sa)->get_outbound_state(*winning_sa) != CHILD_OUTBOUND_INSTALLED)
				{
					charon->bus->child_rekey(charon->bus, this->child_sa,
											 *winning_sa);
				}
			}
			/* check if the peer already sent a delete for the old SA */
			if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
			{
				child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
			}
			else if (this->flags & CHILD_REKEY_OTHER_DELETED)
			{
				/* make sure the old SA is in the correct state if the peer
				 * deleted its own SA but not yet the old one (weird, but who
				 * knows...) */
				this->child_sa->set_state(this->child_sa, CHILD_REKEYED);
			}
		}
		return FALSE;
	}

	*winning_sa = this->child_create->get_child(this->child_create);
	*to_delete = this->child_sa;

	/* regular rekeying without collision (or we already concluded it for a
	 * multi-KE rekeying), check if the peer deleted the new SA already */
	if (!this->collision)
	{
		if (is_our_replacement_deleted(this))
		{
			this->flags |= CHILD_REKEY_REPLACEMENT_DELETED;
			/* since we will destroy the winning SA, we have to trigger a rekey
			 * event before so listeners can track this properly */
			charon->bus->child_rekey(charon->bus, this->child_sa, *winning_sa);
		}
		return TRUE;
	}

	/* the passive rekeying is complete only if it was single-KE.  otherwise,
	 * the peer would either have stopped before sending IKE_FOLLOWUP_KE when
	 * it noticed it lost, or it responded with TEMPORARY_FAILURE to our
	 * CREATE_CHILD_SA request if it already started sending them. */
	if (this->flags & CHILD_REKEY_PASSIVE_INSTALLED)
	{
		if (multi_ke)
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision won, continue with "
				 "multi-KE rekeying");
			/* change the state back, we are not done rekeying yet */
			this->child_sa->set_state(this->child_sa, CHILD_REKEYING);
		}
		else
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision won, deleting old child "
				 "%s{%u}", (*to_delete)->get_name(*to_delete),
				 (*to_delete)->get_unique_id(*to_delete));
		}

		other_sa = other->child_create->get_child(other->child_create);

		/* check if the peer already sent a delete for our winning SA */
		if (is_our_replacement_deleted(this))
		{
			this->flags |= CHILD_REKEY_REPLACEMENT_DELETED;
			/* similar to the case above, but here the peer might already have
			 * deleted its redundant SA, and it might have sent an incorrect
			 * delete for the old SA. if it did the latter first, then we will
			 * have concluded the rekeying and there was a rekey event from the
			 * old SA to the redundant one that we have to consider here */
			if (this->flags & CHILD_REKEY_OLD_SA_DELETED && other_sa &&
				other_sa->get_outbound_state(other_sa) == CHILD_OUTBOUND_INSTALLED)
			{
				charon->bus->child_rekey(charon->bus, other_sa, *winning_sa);
			}
			else
			{
				charon->bus->child_rekey(charon->bus, this->child_sa,
										 *winning_sa);
			}
		}

		/* check if the peer already sent a delete for its redundant SA */
		if (!(this->flags & CHILD_REKEY_OTHER_DELETED))
		{
			/* unlink the redundant SA the peer is expected to delete, disable
			 * events and make sure the outbound SA isn't installed/registered */
			this->child_sa->set_rekey_sa(this->child_sa, NULL);
			if (other_sa)
			{
				other_sa->set_rekey_sa(other_sa, NULL);
				other_sa->set_state(other_sa, CHILD_REKEYED);
				other_sa->remove_outbound(other_sa);
			}
		}
		else if (other_sa)
		{
			/* the peer already deleted its redundant SA, but we have not yet
			 * destroyed it, do so now */
			child_delete_destroy_rekeyed(this->ike_sa, other_sa);
		}
		this->collision->destroy(this->collision);
	}
	else
	{
		/* the peer will not continue with its multi-KE rekeying, so we must
		 * remove the passive task that's waiting for IKE_FOLLOWUP_KEs */
		if (multi_ke)
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision won, continue with "
				 "multi-KE rekeying and remove passive %N task",
				 task_type_names, TASK_CHILD_REKEY);
		}
		else
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision won, remove passive %N "
				 "task", task_type_names, TASK_CHILD_REKEY);
		}
		remove_passive_rekey_task(this);
	}
	this->collision = NULL;
	return TRUE;
}

/**
 * Check if we can ignore a CHILD_SA_NOT_FOUND notify and log appropriate
 * messages.
 */
static bool ignore_child_sa_not_found(private_child_rekey_t *this)
{
	private_child_rekey_t *other;
	child_sa_t *other_sa;

	/* if the peer hasn't explicitly sent a delete for the CHILD_SA it wasn't
	 * able to find now, it might have lost the state, we can't ignore that and
	 * create a replacement */
	if (!(this->flags & CHILD_REKEY_OLD_SA_DELETED))
	{
		DBG1(DBG_IKE, "peer didn't find CHILD_SA %s{%u} we tried to rekey, "
			 "create a replacement",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		return FALSE;
	}

	/* if the peer explicitly deleted the original CHILD_SA before our request
	 * arrived, we adhere to that wish and close the SA.
	 * this is the case where the peer received the DELETE response before
	 * our rekey request, see below for the case where it hasn't received the
	 * response yet and responded with TEMPORARY_FAILURE */
	if (!this->collision)
	{
		DBG1(DBG_IKE, "closing CHILD_SA %s{%u} we tried to rekey because "
			 "the peer deleted it before it received our request",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		child_delete_destroy_and_reestablish(this->ike_sa, this->child_sa);
		return TRUE;
	}

	/* if there was a rekey collision and the peer deleted the original CHILD_SA
	 * before our request arrived and it has not deleted the new SA, we just
	 * abort our own rekeying and use the peer's replacement */
	if (!(this->flags & CHILD_REKEY_OTHER_DELETED))
	{
		DBG1(DBG_IKE, "abort active rekeying for CHILD_SA %s{%u} because "
			 "it was successfully rekeyed by the peer before it received "
			 "our request", this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
		return TRUE;
	}

	/* the peer successfully rekeyed the same SA, deleted it, but then also
	 * deleted the CHILD_SA it created as replacement. adhere to that wish and
	 * close the replacement */
	other = (private_child_rekey_t*)this->collision;
	other_sa = other->child_create->get_child(other->child_create);

	DBG1(DBG_IKE, "abort active rekeying for CHILD_SA %s{%u} because the other "
		 "peer already deleted its replacement CHILD_SA %s{%u} before "
		 "it received our request", this->child_sa->get_name(this->child_sa),
		 this->child_sa->get_unique_id(this->child_sa),
		 other_sa->get_name(other_sa), other_sa->get_unique_id(other_sa));
	child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
	child_delete_destroy_and_reestablish(this->ike_sa, other_sa);
	return TRUE;
}

/**
 * Check if we can ignore failures to create the new CHILD_SA e.g. due to an
 * error notify like TEMPORARY_FAILURE and log appropriate messages.
 */
static bool ignore_child_sa_failure(private_child_rekey_t *this)
{
	/* we are fine if there was a successful passive rekeying. the peer might
	 * not have detected the collision and responded with a TEMPORARY_FAILURE
	 * notify while deleting the old SA, which conflicted with our request */
	if (this->collision && (this->flags & CHILD_REKEY_PASSIVE_INSTALLED) &&
		!(this->flags & CHILD_REKEY_OTHER_DELETED))
	{
		DBG1(DBG_IKE, "abort active rekeying for CHILD_SA %s{%u} because "
			 "the peer successfully rekeyed it before receiving our request%s",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa),
			 this->flags & CHILD_REKEY_OLD_SA_DELETED ? ""
													  : ", waiting for delete");

		/* if the peer already deleted the rekeyed SA, destroy it, otherwise
		 * just wait for the delete */
		if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
		{
			child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
		}
		return TRUE;
	}

	/* if the peer initiated a delete for the old SA before our rekey request
	 * reached it, the expected response is TEMPORARY_FAILURE.  adhere to that
	 * wish and abort the rekeying.
	 * this is the case where the peer has not yet received the DELETE response
	 * when our rekey request arrived, see above for the case where it has
	 * already received the response and responded with CHILD_SA_NOT_FOUND */
	if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
	{
		DBG1(DBG_IKE, "closing CHILD_SA %s{%u} we tried to rekey because "
			 "the peer started to delete it before receiving our request",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		child_delete_destroy_and_reestablish(this->ike_sa, this->child_sa);
		return TRUE;
	}
	return FALSE;
}

/**
 * Check if we can ignore local failures to create the new CHILD_SA e.g. due to
 * a KE or kernel problem and log an appropriate message.
 */
static status_t handle_local_failure(private_child_rekey_t *this)
{
	/* if we lost the collision, we are expected to delete the failed SA
	 * anyway, so just do that and rely on the passive rekeying, which
	 * deletes the old SA (or has already done so, in which case we destroy the
	 * SA now) */
	if (this->collision && lost_collision(this))
	{
		if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
		{
			child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
		}
		this->public.task.build = _build_i_delete_replacement;
		return NEED_MORE;
	}

	/* the peer sent a delete for our winning replacement SA, no need to send a
	 * delete for it again and adhere to this wish to delete the SA.
	 * however, we are expected to send a delete for the original SA, unless,
	 * it was already deleted by the peer as well (which would be incorrect) */
	if (is_our_replacement_deleted(this))
	{
		DBG1(DBG_IKE, "closing CHILD_SA %s{%u} we tried to rekey because "
			 "the peer meanwhile sent a delete for its replacement",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
		{
			child_delete_destroy_and_reestablish(this->ike_sa, this->child_sa);
			return SUCCESS;
		}
		this->public.task.build = _build_i_delete_old_destroy;
		return NEED_MORE;
	}

	/* as the winner of the collision or if there wasn't one, we're expected to
	 * delete the original SA, but we also want to recreate it because we
	 * failed to install the replacement.  because the peer already has the
	 * replacement partially installed, we also need to send a delete for the
	 * failed one */
	this->public.task.build = _build_i_delete_both_recreate;

	if (this->flags & CHILD_REKEY_OLD_SA_DELETED)
	{
		/* the peer already sent an incorrect delete for the original SA that
		 * arrived before the response to the rekeying, delete only the failed
		 * replacement and recreate the SA */
		DBG1(DBG_IKE, "peer sent an incorrect delete for CHILD_SA %s{%u} after "
			 "responding to our rekeying",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		this->public.task.build = _build_i_delete_replacement_recreate;
	}
	DBG1(DBG_IKE, "closing and recreating CHILD_SA %s{%u} after failing to "
		 "install replacement", this->child_sa->get_name(this->child_sa),
		 this->child_sa->get_unique_id(this->child_sa));
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_child_rekey_t *this, message_t *message)
{
	protocol_id_t protocol;
	uint32_t spi;
	child_sa_t *child_sa, *to_delete = NULL, *winning_sa = NULL;
	bool collision_won;

	if (message->get_notify(message, NO_ADDITIONAL_SAS))
	{
		DBG1(DBG_IKE, "peer seems to not support CHILD_SA rekeying, "
			 "starting reauthentication");
		this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
		lib->processor->queue_job(lib->processor,
				(job_t*)rekey_ike_sa_job_create(
							this->ike_sa->get_id(this->ike_sa), TRUE));
		return SUCCESS;
	}
	if (message->get_notify(message, CHILD_SA_NOT_FOUND))
	{
		/* ignore CHILD_SA_NOT_FOUND error notify in some cases, otherwise
		 * create a replacement SA */
		if (ignore_child_sa_not_found(this))
		{
			return SUCCESS;
		}
		return child_delete_destroy_and_force_reestablish(this->ike_sa,
														  this->child_sa);
	}

	if (this->child_create->task.process(&this->child_create->task,
										 message) == NEED_MORE)
	{
		if (message->get_notify(message, INVALID_KE_PAYLOAD))
		{
			/* invalid KE method => retry, unless we can ignore it */
			return ignore_child_sa_failure(this) ? SUCCESS : NEED_MORE;
		}
		else if (!this->child_create->get_child(this->child_create))
		{
			/* local failure requiring a delete, check what we have to do */
			return handle_local_failure(this);
		}

		/* multiple key exchanges */
		this->flags |= CHILD_REKEY_FOLLOWUP_KE;
		/* there will only be a collision while we process a CREATE_CHILD_SA
		 * response, later we just respond with TEMPORARY_FAILURE, so handle
		 * it now */
		if (!handle_collision(this, &to_delete, &winning_sa, TRUE))
		{
			/* we lost the collision. since the SA is not complete yet, we just
			 * abort the task */
			return SUCCESS;
		}
		return NEED_MORE;
	}

	child_sa = this->child_create->get_child(this->child_create);
	if (!child_sa || child_sa->get_state(child_sa) != CHILD_INSTALLED)
	{
		/* check if we can ignore remote errors like TEMPORARY_FAILURE */
		if (!ignore_child_sa_failure(this))
		{
			/* otherwise (e.g. for an IKE/CHILD rekey collision), reuse the old
			 * CHILD_SA and try again */
			schedule_delayed_rekey(this);
		}
		return SUCCESS;
	}

	/* there won't be a collision if this task is for a multi-KE rekeying, as a
	 * collision during CREATE_CHILD_SA was cleaned up above */
	collision_won = handle_collision(this, &to_delete, &winning_sa, FALSE);

	if (this->flags & CHILD_REKEY_REPLACEMENT_DELETED)
	{
		DBG1(DBG_IKE, "peer meanwhile sent a delete for CHILD_SA %s{%u} with "
			 "SPIs %.8x_i %.8x_o, abort rekeying",
			 winning_sa->get_name(winning_sa),
			 winning_sa->get_unique_id(winning_sa),
			 ntohl(winning_sa->get_spi(winning_sa, TRUE)),
			 ntohl(winning_sa->get_spi(winning_sa, FALSE)));
		child_delete_destroy_and_reestablish(this->ike_sa, winning_sa);
	}
	else if (collision_won)
	{
		/* only conclude the rekeying here if we won, otherwise, we either
		 * already concluded the rekeying or we will do so when the peer deletes
		 * the old SA */
		child_rekey_conclude_rekeying(this->child_sa, winning_sa);
	}

	if (collision_won &&
		this->flags & CHILD_REKEY_OLD_SA_DELETED)
	{
		/* the peer already deleted the rekeyed SA we were expected to delete
		 * with an incorrect delete to which we responded as usual but didn't
		 * destroy the SA yet */
		DBG1(DBG_IKE, "peer sent an incorrect delete for CHILD_SA %s{%u} after "
			 "responding to our rekeying",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		child_delete_destroy_rekeyed(this->ike_sa, this->child_sa);
		return SUCCESS;
	}

	/* disable updown event for old/redundant CHILD_SA */
	to_delete->set_state(to_delete, CHILD_REKEYED);
	/* and make sure the outbound SA is not registered, unless it is still fully
	 * installed, which happens if the rekeying is aborted. we keep it installed
	 * as we can't establish a replacement until the delete is done */
	if (to_delete->get_outbound_state(to_delete) != CHILD_OUTBOUND_INSTALLED)
	{
		to_delete->remove_outbound(to_delete);
	}

	spi = to_delete->get_spi(to_delete, TRUE);
	protocol = to_delete->get_protocol(to_delete);

	/* rekeying done, delete the obsolete CHILD_SA using a subtask */
	this->child_delete = child_delete_create(this->ike_sa, protocol, spi, FALSE);
	this->public.task.build = _build_i_delete;
	this->public.task.process = _process_i_delete;

	return NEED_MORE;
}

/*
 * Described in header
 */
bool child_rekey_conclude_rekeying(child_sa_t *old, child_sa_t *new)
{
	linked_list_t *my_ts, *other_ts;

	if (new->install_outbound(new) != SUCCESS)
	{
		/* shouldn't happen after we were able to install the inbound SA */
		DBG1(DBG_IKE, "unable to install outbound IPsec SA (SAD) in kernel");
		charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_SA_FAILED,
						   new);
		return FALSE;
	}

	my_ts = linked_list_create_from_enumerator(
							new->create_ts_enumerator(new, TRUE));
	other_ts = linked_list_create_from_enumerator(
							new->create_ts_enumerator(new, FALSE));

	DBG0(DBG_IKE, "outbound CHILD_SA %s{%d} established "
		 "with SPIs %.8x_i %.8x_o and TS %#R === %#R",
		 new->get_name(new), new->get_unique_id(new),
		 ntohl(new->get_spi(new, TRUE)), ntohl(new->get_spi(new, FALSE)),
		 my_ts, other_ts);

	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);

	/* remove the old outbound SA after we installed the new one. otherwise, it
	 * might not get used yet depending on how SAs/policies are handled in the
	 * kernel */
	old->remove_outbound(old);

	DBG0(DBG_IKE, "rekeyed CHILD_SA %s{%u} with SPIs %.8x_i %.8x_o with "
		 "%s{%u} with SPIs %.8x_i %.8x_o",
		 old->get_name(old), old->get_unique_id(old),
		 ntohl(old->get_spi(old, TRUE)), ntohl(old->get_spi(old, FALSE)),
		 new->get_name(new), new->get_unique_id(new),
		 ntohl(new->get_spi(new, TRUE)), ntohl(new->get_spi(new, FALSE)));
	charon->bus->child_rekey(charon->bus, old, new);
	return TRUE;
}

METHOD(task_t, get_type, task_type_t,
	private_child_rekey_t *this)
{
	return TASK_CHILD_REKEY;
}

METHOD(child_rekey_t, handle_delete, child_rekey_collision_t,
	private_child_rekey_t *this, child_sa_t *child, uint32_t spi)
{
	/* if we already completed our active rekeying and are deleting the
	 * old/redundant SA, there is no need to do anything special */
	if (this->child_delete)
	{
		return CHILD_REKEY_COLLISION_NONE;
	}

	if (!child)
	{
		/* check later if the SPI is the peer's of the SA we created (i.e.
		 * whether it deleted the new SA immediately after creation and we
		 * received that request before our active rekeying was complete) */
		array_insert_create_value(&this->deleted_spis, sizeof(uint32_t),
								  ARRAY_TAIL, &spi);
	}
	else if (child == this->child_sa)
	{
		/* the peer sent a delete for the old SA, might be because it won a
		 * collision, but could also be either because it initiated that before
		 * it received our CREATE_CHILD_SA request, or it incorrectly sent one
		 * as response to our request, we will check once we have the response
		 * to our rekeying */
		this->flags |= CHILD_REKEY_OLD_SA_DELETED;
		return CHILD_REKEY_COLLISION_OLD;
	}
	else if (this->collision)
	{
		private_child_rekey_t *other = (private_child_rekey_t*)this->collision;

		if (child == other->child_create->get_child(other->child_create))
		{
			/* the peer deleted the redundant (or in rare cases the winning) SA
			 * it created before our active rekeying was complete, how we handle
			 * this depends on the response to our rekeying */
			this->flags |= CHILD_REKEY_OTHER_DELETED;
			return CHILD_REKEY_COLLISION_PEER;
		}
	}
	return CHILD_REKEY_COLLISION_NONE;
}

METHOD(child_rekey_t, collide, bool,
	private_child_rekey_t *this, task_t *other)
{
	private_child_rekey_t *rekey = (private_child_rekey_t*)other;
	child_sa_t *other_child;

	if (rekey->child_sa != this->child_sa)
	{
		/* not the same child => no collision */
		return FALSE;
	}

	other_child = rekey->child_create->get_child(rekey->child_create);
	if (!other_child)
	{
		/* ignore passive tasks that did not successfully create a CHILD_SA */
		return FALSE;
	}
	if (other_child->get_state(other_child) != CHILD_INSTALLED)
	{
		DBG1(DBG_IKE, "colliding passive rekeying for CHILD_SA %s{%u} is not "
			 "yet complete", this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		/* we do reference the task to check its state later */
		this->collision = other;
		return FALSE;
	}
	if (this->collision && this->collision != other)
	{
		DBG1(DBG_IKE, "duplicate rekey collision for CHILD_SA %s{%u}???",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
		return FALSE;
	}
	/* once the passive rekeying is complete, we adopt the task */
	DBG1(DBG_IKE, "detected rekey collision for CHILD_SA %s{%u}",
		 this->child_sa->get_name(this->child_sa),
		 this->child_sa->get_unique_id(this->child_sa));
	this->flags |= CHILD_REKEY_PASSIVE_INSTALLED;
	this->collision = other;
	return TRUE;
}

METHOD(task_t, migrate, void,
	private_child_rekey_t *this, ike_sa_t *ike_sa)
{
	/* only migrate the currently active task */
	if (this->child_delete)
	{
		this->child_delete->task.migrate(&this->child_delete->task, ike_sa);
	}
	else if (this->child_create)
	{
		this->child_create->task.migrate(&this->child_create->task, ike_sa);
	}
	if (this->flags & CHILD_REKEY_PASSIVE_INSTALLED)
	{
		DESTROY_IF(this->collision);
	}
	array_destroy(this->deleted_spis);

	this->ike_sa = ike_sa;
	this->collision = NULL;
	this->flags = 0;
}

METHOD(task_t, destroy, void,
	private_child_rekey_t *this)
{
	if (this->child_create)
	{
		this->child_create->task.destroy(&this->child_create->task);
	}
	if (this->child_delete)
	{
		this->child_delete->task.destroy(&this->child_delete->task);
	}
	if (this->flags & CHILD_REKEY_PASSIVE_INSTALLED)
	{
		DESTROY_IF(this->collision);
	}
	array_destroy(this->deleted_spis);
	chunk_free(&this->spi_data);
	free(this);
}

/*
 * Described in header.
 */
child_rekey_t *child_rekey_create(ike_sa_t *ike_sa, protocol_id_t protocol,
								  uint32_t spi)
{
	private_child_rekey_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.handle_delete = _handle_delete,
			.collide = _collide,
		},
		.ike_sa = ike_sa,
		.protocol = protocol,
		.spi = spi,
	);

	if (protocol != PROTO_NONE)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
		this->initiator = TRUE;
		this->child_create = NULL;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
		this->initiator = FALSE;
		this->child_create = child_create_create(ike_sa, NULL, TRUE,
												 NULL, NULL, 0);
	}

	return &this->public;
}

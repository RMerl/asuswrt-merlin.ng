/*
 * Copyright (C) 2015-2020 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include "ike_rekey.h"

#include <daemon.h>
#include <encoding/payloads/notify_payload.h>
#include <sa/ikev2/tasks/ike_init.h>
#include <sa/ikev2/tasks/ike_delete.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/rekey_ike_sa_job.h>
#include <processing/jobs/initiate_tasks_job.h>

typedef struct private_ike_rekey_t private_ike_rekey_t;

/**
 * Private members of a ike_rekey_t task.
 */
struct private_ike_rekey_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_rekey_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * New IKE_SA which replaces the current one
	 */
	ike_sa_t *new_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * the TASK_IKE_INIT task which is reused to simplify rekeying
	 */
	ike_init_t *ike_init;

	/**
	 * IKE_DELETE task to delete the old IKE_SA after rekeying was successful
	 */
	ike_delete_t *ike_delete;

	/**
	 * Colliding passive task if any
	 */
	private_ike_rekey_t *collision;

	/**
	 * Link value for the current key exchange
	 */
	chunk_t link;

	/**
	 * State/error flags
	 */
	enum {

		/**
		 * Set if rekeying can't be handled temporarily.
		 */
		IKE_REKEY_FAILED_TEMPORARILY = (1<<0),

		/**
		 * Set if the parsed link value was invalid.
		 */
		IKE_REKEY_LINK_INVALID = (1<<1),

		/**
		 * Set if we use multiple key exchanges and already processed the
		 * CREATE_CHILD_SA response and started sending IKE_FOLLOWUP_KEs.
		 */
		IKE_REKEY_FOLLOWUP_KE = (1<<2),

		/**
		 * Set if a passive rekeying has completed successfully and we don't
		 * expect any further messages.
		 */
		IKE_REKEY_DONE = (1<<3),

		/**
		 * Set if we adopted a completed passive task, otherwise we just
		 * reference it.
		 */
		IKE_REKEY_ADOPTED_PASSIVE = (1<<4),

	} flags;
};

/**
 * Schedule a retry if rekeying temporary failed
 */
static void schedule_delayed_rekey(private_ike_rekey_t *this)
{
	uint32_t retry;
	job_t *job;

	retry = RETRY_INTERVAL - (random() % RETRY_JITTER);
	job = (job_t*)rekey_ike_sa_job_create(
						this->ike_sa->get_id(this->ike_sa), FALSE);
	DBG1(DBG_IKE, "IKE_SA rekeying failed, trying again in %d seconds", retry);
	this->ike_sa->set_state(this->ike_sa, IKE_ESTABLISHED);
	lib->scheduler->schedule_job(lib->scheduler, job, retry);
}

/**
 * Check if an IKE_SA has any queued tasks, return initiation job
 */
static job_t* check_queued_tasks(ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	task_t *task;
	job_t *job = NULL;

	enumerator = ike_sa->create_task_enumerator(ike_sa, TASK_QUEUE_QUEUED);
	if (enumerator->enumerate(enumerator, &task))
	{
		job = (job_t*)initiate_tasks_job_create(ike_sa->get_id(ike_sa));
	}
	enumerator->destroy(enumerator);
	return job;
}

/**
 * Establish the new replacement IKE_SA
 */
static void establish_new(private_ike_rekey_t *this)
{
	if (this->new_sa)
	{
		job_t *job;

		this->new_sa->set_state(this->new_sa, IKE_ESTABLISHED);
		DBG0(DBG_IKE, "IKE_SA %s[%d] rekeyed between %H[%Y]...%H[%Y]",
			 this->new_sa->get_name(this->new_sa),
			 this->new_sa->get_unique_id(this->new_sa),
			 this->ike_sa->get_my_host(this->ike_sa),
			 this->ike_sa->get_my_id(this->ike_sa),
			 this->ike_sa->get_other_host(this->ike_sa),
			 this->ike_sa->get_other_id(this->ike_sa));

		/* register the new IKE_SA before calling inherit_post() as that may
		 * schedule jobs, as may listeners for ike_rekey() */
		charon->ike_sa_manager->checkout_new(charon->ike_sa_manager,
											 this->new_sa);
		this->new_sa->inherit_post(this->new_sa, this->ike_sa);
		charon->bus->ike_rekey(charon->bus, this->ike_sa, this->new_sa);
		job = check_queued_tasks(this->new_sa);
		if (job)
		{
			lib->processor->queue_job(lib->processor, job);
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, this->new_sa);
		this->new_sa = NULL;
		charon->bus->set_sa(charon->bus, this->ike_sa);

		this->ike_sa->set_state(this->ike_sa, IKE_REKEYED);
	}
}

METHOD(task_t, build_i_delete, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	/* update exchange type to INFORMATIONAL for the delete */
	message->set_exchange_type(message, INFORMATIONAL);

	return this->ike_delete->task.build(&this->ike_delete->task, message);
}

METHOD(task_t, process_i_delete, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	return this->ike_delete->task.process(&this->ike_delete->task, message);
}

METHOD(task_t, build_i_multi_ke, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	status_t status;

	charon->bus->set_sa(charon->bus, this->new_sa);
	message->add_notify(message, FALSE, ADDITIONAL_KEY_EXCHANGE, this->link);
	status = this->ike_init->task.build(&this->ike_init->task, message);
	charon->bus->set_sa(charon->bus, this->ike_sa);
	this->flags |= IKE_REKEY_FOLLOWUP_KE;
	return status;
}

METHOD(task_t, build_i, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	ike_version_t version;
	status_t status;

	/* create new SA only on first try */
	if (!this->new_sa)
	{
		if (this->ike_sa->get_state(this->ike_sa) == IKE_REKEYING ||
			this->ike_sa->get_state(this->ike_sa) == IKE_REKEYED)
		{
			/* ignore SAs that have or are currently being rekeyed passively */
			message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
			return SUCCESS;
		}

		version = this->ike_sa->get_version(this->ike_sa);
		this->new_sa = charon->ike_sa_manager->create_new(
										charon->ike_sa_manager, version, TRUE);
		if (!this->new_sa)
		{	/* shouldn't happen */
			return FAILED;
		}
		this->new_sa->inherit_pre(this->new_sa, this->ike_sa);
		this->ike_init = ike_init_create(this->new_sa, TRUE, this->ike_sa);
		this->ike_sa->set_state(this->ike_sa, IKE_REKEYING);
	}
	status = this->ike_init->task.build(&this->ike_init->task, message);
	charon->bus->set_sa(charon->bus, this->ike_sa);
	return status;
}

/**
 * Check if there are any half-open children
 */
static bool have_half_open_children(private_ike_rekey_t *this)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	task_t *task;

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, (void**)&child_sa))
	{
		switch (child_sa->get_state(child_sa))
		{
			case CHILD_REKEYING:
			case CHILD_RETRYING:
			case CHILD_DELETING:
				enumerator->destroy(enumerator);
				return TRUE;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_ACTIVE);
	while (enumerator->enumerate(enumerator, (void**)&task))
	{
		if (task->get_type(task) == TASK_CHILD_CREATE)
		{
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

/**
 * Check if we are actively rekeying and optionally, if we already sent an
 * IKE_FOLLOWUP_KE message.
 */
static bool actively_rekeying(private_ike_rekey_t *this, bool *follow_up_sent)
{
	enumerator_t *enumerator;
	task_t *task;
	bool found = FALSE;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_ACTIVE);
	while (enumerator->enumerate(enumerator, (void**)&task))
	{
		if (task->get_type(task) == TASK_IKE_REKEY)
		{
			if (follow_up_sent)
			{
				private_ike_rekey_t *rekey = (private_ike_rekey_t*)task;
				*follow_up_sent = rekey->flags & IKE_REKEY_FOLLOWUP_KE;
			}
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Process payloads in a IKE_FOLLOWUP_KE message or a CREATE_CHILD_SA response
 */
static void process_link(private_ike_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;
	chunk_t link;

	notify = message->get_notify(message, ADDITIONAL_KEY_EXCHANGE);
	if (!notify)
	{
		DBG1(DBG_IKE, "%N notify missing", notify_type_names,
			ADDITIONAL_KEY_EXCHANGE);
		this->flags |= IKE_REKEY_LINK_INVALID;
	}
	else
	{
		link = notify->get_notification_data(notify);
		if (this->initiator)
		{
			chunk_free(&this->link);
			this->link = chunk_clone(link);
		}
		else if (!chunk_equals_const(this->link, link))
		{
			DBG1(DBG_IKE, "data in %N notify doesn't match", notify_type_names,
				 ADDITIONAL_KEY_EXCHANGE);
			this->flags |= IKE_REKEY_LINK_INVALID;
		}
	}
}

METHOD(task_t, process_r_multi_ke, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	if (message->get_exchange_type(message) != IKE_FOLLOWUP_KE)
	{
		return FAILED;
	}
	if (this->ike_sa->get_state(this->ike_sa) == IKE_DELETING)
	{
		DBG1(DBG_IKE, "peer continued rekeying, but we are deleting");
		this->flags |= IKE_REKEY_FAILED_TEMPORARILY;
		return NEED_MORE;
	}

	charon->bus->set_sa(charon->bus, this->new_sa);
	process_link(this, message);
	this->ike_init->task.process(&this->ike_init->task, message);
	charon->bus->set_sa(charon->bus, this->ike_sa);
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	bool follow_up_sent;

	if (this->ike_sa->get_state(this->ike_sa) == IKE_DELETING)
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but we are deleting");
		this->flags |= IKE_REKEY_FAILED_TEMPORARILY;
		return NEED_MORE;
	}
	if (this->ike_sa->has_condition(this->ike_sa, COND_REAUTHENTICATING))
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but we are reauthenticating");
		this->flags |= IKE_REKEY_FAILED_TEMPORARILY;
		return NEED_MORE;
	}
	if (have_half_open_children(this))
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but a child is half-open");
		this->flags |= IKE_REKEY_FAILED_TEMPORARILY;
		return NEED_MORE;
	}
	if (actively_rekeying(this, &follow_up_sent) && follow_up_sent)
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but we did too and already "
			 "sent IKE_FOLLOWUP_KE");
		this->flags |= IKE_REKEY_FAILED_TEMPORARILY;
		return NEED_MORE;
	}

	this->new_sa = charon->ike_sa_manager->create_new(charon->ike_sa_manager,
							this->ike_sa->get_version(this->ike_sa), FALSE);
	if (!this->new_sa)
	{	/* shouldn't happen */
		return FAILED;
	}
	this->new_sa->inherit_pre(this->new_sa, this->ike_sa);
	this->ike_init = ike_init_create(this->new_sa, FALSE, this->ike_sa);
	this->ike_init->task.process(&this->ike_init->task, message);
	charon->bus->set_sa(charon->bus, this->ike_sa);
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	if (this->flags & IKE_REKEY_FAILED_TEMPORARILY)
	{
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}
	if (this->flags & IKE_REKEY_LINK_INVALID)
	{
		message->add_notify(message, TRUE, STATE_NOT_FOUND, chunk_empty);
		return SUCCESS;
	}
	if (!this->new_sa)
	{
		/* IKE_SA/a CHILD_SA is in an unacceptable state, deny rekeying */
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return SUCCESS;
	}

	charon->bus->set_sa(charon->bus, this->new_sa);
	switch (this->ike_init->task.build(&this->ike_init->task, message))
	{
		case FAILED:
			this->ike_init->task.destroy(&this->ike_init->task);
			this->ike_init = NULL;
			charon->bus->set_sa(charon->bus, this->ike_sa);
			return SUCCESS;
		case NEED_MORE:
			/* additional key exchanges, the value in the notify doesn't really
			 * matter to us as we have a window size of 1 */
			charon->bus->set_sa(charon->bus, this->ike_sa);
			if (!this->link.ptr)
			{
				this->link = chunk_clone(chunk_from_chars(0x42));
			}
			message->add_notify(message, FALSE, ADDITIONAL_KEY_EXCHANGE,
								this->link);
			if (this->ike_sa->get_state(this->ike_sa) != IKE_REKEYING)
			{
				this->ike_sa->set_state(this->ike_sa, IKE_REKEYING);
			}
			this->public.task.process = _process_r_multi_ke;
			return NEED_MORE;
		default:
			charon->bus->set_sa(charon->bus, this->ike_sa);
			if (this->ike_sa->get_state(this->ike_sa) != IKE_REKEYING)
			{
				this->ike_sa->set_state(this->ike_sa, IKE_REKEYING);
			}
			break;
	}

	this->flags |= IKE_REKEY_DONE;

	/* if we are actively rekeying, we let the initiating task handle this */
	if (!actively_rekeying(this, NULL))
	{
		establish_new(this);
		/* make sure the IKE_SA is gone in case the peer fails to delete it */
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
			delete_ike_sa_job_create(this->ike_sa->get_id(this->ike_sa), TRUE),
									 90);
	}
	return SUCCESS;
}

/**
 * Conclude any (undetected) rekey collision.
 *
 * If the peer does not detect the collision it will delete this IKE_SA.
 * Depending on when our request reaches the peer and we receive the delete
 * this may get called at different times.
 *
 * Returns TRUE if there was a collision, FALSE otherwise.
 */
static bool conclude_collision(private_ike_rekey_t *this, bool maybe_undetected)
{
	if (this->collision &&
		this->flags & IKE_REKEY_ADOPTED_PASSIVE)
	{
		if (maybe_undetected)
		{
			DBG1(DBG_IKE, "peer may not have noticed IKE_SA rekey collision, "
				 "abort active rekeying");
		}
		establish_new(this->collision);
		return TRUE;
	}
	return FALSE;
}

/**
 * Delete the redundant IKE_SA we created.
 */
static void delete_redundant(private_ike_rekey_t *this)
{
	host_t *host;

	/* apply host for a proper delete */
	host = this->ike_sa->get_my_host(this->ike_sa);
	this->new_sa->set_my_host(this->new_sa, host->clone(host));
	host = this->ike_sa->get_other_host(this->ike_sa);
	this->new_sa->set_other_host(this->new_sa, host->clone(host));
	this->new_sa->set_state(this->new_sa, IKE_REKEYED);
	if (this->new_sa->delete(this->new_sa, FALSE) == DESTROY_ME)
	{
		this->new_sa->destroy(this->new_sa);
	}
	else
	{
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, this->new_sa);
	}
	charon->bus->set_sa(charon->bus, this->ike_sa);
	this->new_sa = NULL;
}

/**
 * Check in the redundant IKE_SA created by the peer and wait for its deletion.
 */
static void wait_for_redundant_delete(private_ike_rekey_t *this)
{
	private_ike_rekey_t *other = this->collision;
	job_t *job;

	/* peer should delete the SA it created, add a timeout just in case */
	job = (job_t*)delete_ike_sa_job_create(
								other->new_sa->get_id(other->new_sa), TRUE);
	lib->scheduler->schedule_job(lib->scheduler, job, HALF_OPEN_IKE_SA_TIMEOUT);
	other->new_sa->set_state(other->new_sa, IKE_REKEYED);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, other->new_sa);
	other->new_sa = NULL;
	charon->bus->set_sa(charon->bus, this->ike_sa);
}

/**
 * Remove the passive rekey task that's waiting for IKE_FOLLOWUP_KE requests
 * that will never come.
 */
static void remove_passive_rekey_task(private_ike_rekey_t *this)
{
	enumerator_t *enumerator;
	task_t *task;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_PASSIVE);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_IKE_REKEY)
		{
			this->ike_sa->remove_task(this->ike_sa, enumerator);
			task->destroy(task);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Handle any collision as necessary and report back if we lost the
 * collision and should abort the active task.
 */
static bool collision_lost(private_ike_rekey_t *this, bool multi_ke)
{
	private_ike_rekey_t *other = this->collision;
	chunk_t this_nonce, other_nonce;

	if (!this->collision)
	{
		return FALSE;
	}

	this_nonce = this->ike_init->get_lower_nonce(this->ike_init);
	other_nonce = other->ike_init->get_lower_nonce(other->ike_init);

	/* the SA with the lowest nonce should be deleted (if already complete),
	 * check if we or the peer created that */
	if (memcmp(this_nonce.ptr, other_nonce.ptr,
			   min(this_nonce.len, other_nonce.len)) < 0)
	{
		if (multi_ke)
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision lost, abort incomplete "
				 "multi-KE rekeying");
		}
		else
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision lost, deleting redundant "
				 "IKE_SA %s[%d]", this->new_sa->get_name(this->new_sa),
				 this->new_sa->get_unique_id(this->new_sa));
			delete_redundant(this);
		}
		/* establish the other SA if the passive task is done (i.e. was
		 * single-KE or our response was delayed and the winner continued),
		 * otherwise, we just let it continue independently */
		conclude_collision(this, FALSE);
		return TRUE;
	}

	/* the passive rekeying is complete only if it was single-KE. otherwise,
	 * the peer would either have stopped before sending IKE_FOLLOWUP_KE when it
	 * noticed it lost, or it responded with TEMPORARY_FAILURE to our
	 * CREATE_CHILD_SA request if it already started sending them.
	 * since the task is not completed immediately, we clean up the collision */
	if (this->flags & IKE_REKEY_ADOPTED_PASSIVE)
	{
		if (multi_ke)
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision won, continue with multi-KE "
				 "rekeying and wait for delete for redundant IKE_SA %s[%d]",
				 other->new_sa->get_name(other->new_sa),
				 other->new_sa->get_unique_id(other->new_sa));
		}
		else
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision won, waiting for delete for "
				 "redundant IKE_SA %s[%d]",
				 other->new_sa->get_name(other->new_sa),
				 other->new_sa->get_unique_id(other->new_sa));
		}
		wait_for_redundant_delete(this);
		other->public.task.destroy(&other->public.task);
	}
	else
	{
		/* the peer will not continue with its multi-KE rekeying, so we must
		 * remove the passive task that's waiting for IKE_FOLLOWUP_KEs */
		if (multi_ke)
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision won, continue with "
				 "multi-KE rekeying and remove passive %N task",
				 task_type_names, TASK_IKE_REKEY);
		}
		else
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision won, remove passive %N task",
				 task_type_names, TASK_IKE_REKEY);
		}
		remove_passive_rekey_task(this);
	}
	this->collision = NULL;
	return FALSE;
}

METHOD(task_t, process_i, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	if (message->get_notify(message, NO_ADDITIONAL_SAS))
	{
		DBG1(DBG_IKE, "peer seems to not support IKE rekeying, "
			 "starting reauthentication");
		this->ike_sa->set_state(this->ike_sa, IKE_ESTABLISHED);
		lib->processor->queue_job(lib->processor,
				(job_t*)rekey_ike_sa_job_create(
							this->ike_sa->get_id(this->ike_sa), TRUE));
		return SUCCESS;
	}
	if (message->get_notify(message, STATE_NOT_FOUND))
	{
		DBG1(DBG_IKE, "peer didn't like our %N notify data", notify_type_names,
			 ADDITIONAL_KEY_EXCHANGE);
		if (!conclude_collision(this, TRUE))
		{
			schedule_delayed_rekey(this);
		}
		return SUCCESS;
	}

	charon->bus->set_sa(charon->bus, this->new_sa);
	switch (this->ike_init->task.process(&this->ike_init->task, message))
	{
		case FAILED:
			charon->bus->set_sa(charon->bus, this->ike_sa);
			/* rekeying failed, fallback to old SA */
			if (!conclude_collision(this, TRUE))
			{
				schedule_delayed_rekey(this);
			}
			return SUCCESS;
		case NEED_MORE:
			if (message->get_notify(message, INVALID_KE_PAYLOAD))
			{	/* bad key exchange mechanism, try again */
				this->ike_init->task.migrate(&this->ike_init->task,
											 this->new_sa);
				charon->bus->set_sa(charon->bus, this->ike_sa);
				return NEED_MORE;
			}
			/* multiple key exchanges, continue with IKE_FOLLOWUP_KE */
			process_link(this, message);
			charon->bus->set_sa(charon->bus, this->ike_sa);
			if (this->flags & IKE_REKEY_LINK_INVALID)
			{	/* we can't continue without notify, maybe the peer returns
				 * one later */
				if (!conclude_collision(this, TRUE))
				{
					schedule_delayed_rekey(this);
				}
				return SUCCESS;
			}
			this->public.task.build = _build_i_multi_ke;
			/* there will only be a collision if we process a CREATE_CHILD_SA
			 * response, if we already sent an IKE_FOLOWUP_KE, the passive task
			 * would just respond with TEMPORARY_FAILURE and get ignored */
			return collision_lost(this, TRUE) ? SUCCESS : NEED_MORE;
		default:
			charon->bus->set_sa(charon->bus, this->ike_sa);
			break;
	}

	/* there will not be a collision here if this task is for a multi-KE
	 * rekeying, as that would already have been handled above when processing
	 * the CREATE_CHILD_SA response */
	if (collision_lost(this, FALSE))
	{
		return SUCCESS;
	}

	establish_new(this);
	/* rekeying successful, delete this IKE_SA using a subtask */
	this->ike_delete = ike_delete_create(this->ike_sa, TRUE);
	this->public.task.build = _build_i_delete;
	this->public.task.process = _process_i_delete;

	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_rekey_t *this)
{
	return TASK_IKE_REKEY;
}

METHOD(ike_rekey_t, did_collide, bool,
	private_ike_rekey_t *this)
{
	return this->collision != NULL;
}

METHOD(ike_rekey_t, collide, bool,
	private_ike_rekey_t* this, task_t *other)
{
	DBG1(DBG_IKE, "detected %N collision with %N", task_type_names,
		 TASK_IKE_REKEY, task_type_names, other->get_type(other));

	switch (other->get_type(other))
	{
		case TASK_IKE_DELETE:
			conclude_collision(this, TRUE);
			break;
		case TASK_IKE_REKEY:
		{
			private_ike_rekey_t *rekey = (private_ike_rekey_t*)other;

			if (!rekey->ike_init)
			{
				DBG1(DBG_IKE, "colliding exchange did not result in an IKE_SA, "
					 "ignore");
				if (this->collision == rekey)
				{
					this->collision = NULL;
				}
				break;
			}
			/* we keep track of the passive exchange in any case, if not
			 * complete yet, this method might be called again later */
			this->collision = rekey;
			if (rekey->flags & IKE_REKEY_DONE)
			{
				this->flags |= IKE_REKEY_ADOPTED_PASSIVE;
				return TRUE;
			}
			DBG1(DBG_IKE, "colliding passive exchange is not yet complete");
			break;
		}
		default:
			/* shouldn't happen */
			break;
	}
	return FALSE;
}

/**
 * Cleanup the task
 */
static void cleanup(private_ike_rekey_t *this)
{
	ike_sa_t *cur_sa;

	if (this->ike_init)
	{
		this->ike_init->task.destroy(&this->ike_init->task);
	}
	if (this->ike_delete)
	{
		this->ike_delete->task.destroy(&this->ike_delete->task);
	}
	cur_sa = charon->bus->get_sa(charon->bus);
	DESTROY_IF(this->new_sa);
	charon->bus->set_sa(charon->bus, cur_sa);
	/* only destroy if the passive task was adopted, otherwise it is still
	 * queued and might get destroyed by the task manager */
	if (this->collision &&
		this->flags & IKE_REKEY_ADOPTED_PASSIVE)
	{
		this->collision->public.task.destroy(&this->collision->public.task);
	}
	chunk_free(&this->link);
}

METHOD(task_t, migrate, void,
	private_ike_rekey_t *this, ike_sa_t *ike_sa)
{
	cleanup(this);
	this->collision = NULL;
	this->ike_sa = ike_sa;
	this->new_sa = NULL;
	this->ike_init = NULL;
	this->ike_delete = NULL;
	this->flags = 0;
}

METHOD(task_t, destroy, void,
	private_ike_rekey_t *this)
{
	cleanup(this);
	free(this);
}

/*
 * Described in header.
 */
ike_rekey_t *ike_rekey_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_rekey_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.build = _build_r,
				.process = _process_r,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.did_collide = _did_collide,
			.collide = _collide,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
	);
	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}

	return &this->public;
}

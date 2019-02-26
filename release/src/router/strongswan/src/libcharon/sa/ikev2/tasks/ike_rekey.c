/*
 * Copyright (C) 2015-2016 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
	 * colliding task detected by the task manager
	 */
	task_t *collision;

	/**
	 * TRUE if rekeying can't be handled temporarily
	 */
	bool failed_temporarily;
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

		this->new_sa->inherit_post(this->new_sa, this->ike_sa);
		charon->bus->ike_rekey(charon->bus, this->ike_sa, this->new_sa);
		job = check_queued_tasks(this->new_sa);
		/* don't queue job before checkin(), as the IKE_SA is not yet
		 * registered at the manager */
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, this->new_sa);
		if (job)
		{
			lib->processor->queue_job(lib->processor, job);
		}
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

METHOD(task_t, build_i, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	ike_version_t version;

	/* create new SA only on first try */
	if (this->new_sa == NULL)
	{
		version = this->ike_sa->get_version(this->ike_sa);
		this->new_sa = charon->ike_sa_manager->checkout_new(
										charon->ike_sa_manager, version, TRUE);
		if (!this->new_sa)
		{	/* shouldn't happen */
			return FAILED;
		}
		this->new_sa->inherit_pre(this->new_sa, this->ike_sa);
		this->ike_init = ike_init_create(this->new_sa, TRUE, this->ike_sa);
		this->ike_sa->set_state(this->ike_sa, IKE_REKEYING);
	}
	this->ike_init->task.build(&this->ike_init->task, message);

	return NEED_MORE;
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

METHOD(task_t, process_r, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	if (this->ike_sa->get_state(this->ike_sa) == IKE_DELETING)
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but we are deleting");
		this->failed_temporarily = TRUE;
		return NEED_MORE;
	}
	if (have_half_open_children(this))
	{
		DBG1(DBG_IKE, "peer initiated rekeying, but a child is half-open");
		this->failed_temporarily = TRUE;
		return NEED_MORE;
	}

	this->new_sa = charon->ike_sa_manager->checkout_new(charon->ike_sa_manager,
							this->ike_sa->get_version(this->ike_sa), FALSE);
	if (!this->new_sa)
	{	/* shouldn't happen */
		return FAILED;
	}
	this->new_sa->inherit_pre(this->new_sa, this->ike_sa);
	this->ike_init = ike_init_create(this->new_sa, FALSE, this->ike_sa);
	this->ike_init->task.process(&this->ike_init->task, message);

	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_rekey_t *this, message_t *message)
{
	if (this->failed_temporarily)
	{
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}
	if (this->new_sa == NULL)
	{
		/* IKE_SA/a CHILD_SA is in an unacceptable state, deny rekeying */
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return SUCCESS;
	}
	if (this->ike_init->task.build(&this->ike_init->task, message) == FAILED)
	{
		this->ike_init->task.destroy(&this->ike_init->task);
		this->ike_init = NULL;
		charon->bus->set_sa(charon->bus, this->ike_sa);
		return SUCCESS;
	}
	charon->bus->set_sa(charon->bus, this->ike_sa);

	if (this->ike_sa->get_state(this->ike_sa) != IKE_REKEYING)
	{	/* in case of a collision we let the initiating task handle this */
		establish_new(this);
		/* make sure the IKE_SA is gone in case the peer fails to delete it */
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
			delete_ike_sa_job_create(this->ike_sa->get_id(this->ike_sa), TRUE),
									 90);
	}
	return SUCCESS;
}

/**
 * Conclude any undetected rekey collision.
 *
 * If the peer does not detect the collision it will delete this IKE_SA.
 * Depending on when our request reaches the peer and we receive the delete
 * this may get called at different times.
 *
 * Returns TRUE if there was a collision, FALSE otherwise.
 */
static bool conclude_undetected_collision(private_ike_rekey_t *this)
{
	if (this->collision &&
		this->collision->get_type(this->collision) == TASK_IKE_REKEY)
	{
		DBG1(DBG_IKE, "peer did not notice IKE_SA rekey collision, abort "
			 "active rekeying");
		establish_new((private_ike_rekey_t*)this->collision);
		return TRUE;
	}
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

	switch (this->ike_init->task.process(&this->ike_init->task, message))
	{
		case FAILED:
			/* rekeying failed, fallback to old SA */
			if (!conclude_undetected_collision(this))
			{
				schedule_delayed_rekey(this);
			}
			return SUCCESS;
		case NEED_MORE:
			/* bad dh group, try again */
			this->ike_init->task.migrate(&this->ike_init->task, this->new_sa);
			return NEED_MORE;
		default:
			break;
	}

	/* check for collisions */
	if (this->collision &&
		this->collision->get_type(this->collision) == TASK_IKE_REKEY)
	{
		private_ike_rekey_t *other = (private_ike_rekey_t*)this->collision;
		host_t *host;
		chunk_t this_nonce, other_nonce;

		this_nonce = this->ike_init->get_lower_nonce(this->ike_init);
		other_nonce = other->ike_init->get_lower_nonce(other->ike_init);

		/* if we have the lower nonce, delete rekeyed SA. If not, delete
		 * the redundant. */
		if (memcmp(this_nonce.ptr, other_nonce.ptr,
				   min(this_nonce.len, other_nonce.len)) < 0)
		{
			DBG1(DBG_IKE, "IKE_SA rekey collision lost, deleting redundant "
				 "IKE_SA %s[%d]", this->new_sa->get_name(this->new_sa),
				 this->new_sa->get_unique_id(this->new_sa));
			/* apply host for a proper delete */
			host = this->ike_sa->get_my_host(this->ike_sa);
			this->new_sa->set_my_host(this->new_sa, host->clone(host));
			host = this->ike_sa->get_other_host(this->ike_sa);
			this->new_sa->set_other_host(this->new_sa, host->clone(host));
			/* IKE_SAs in state IKE_REKEYED are silently deleted, so we use
			 * IKE_REKEYING */
			this->new_sa->set_state(this->new_sa, IKE_REKEYING);
			if (this->new_sa->delete(this->new_sa, FALSE) == DESTROY_ME)
			{
				this->new_sa->destroy(this->new_sa);
			}
			else
			{
				charon->ike_sa_manager->checkin(charon->ike_sa_manager,
												this->new_sa);
			}
			charon->bus->set_sa(charon->bus, this->ike_sa);
			this->new_sa = NULL;
			establish_new(other);
			return SUCCESS;
		}
		/* peer should delete this SA. Add a timeout just in case. */
		job_t *job = (job_t*)delete_ike_sa_job_create(
									other->new_sa->get_id(other->new_sa), TRUE);
		lib->scheduler->schedule_job(lib->scheduler, job,
									 HALF_OPEN_IKE_SA_TIMEOUT);
		DBG1(DBG_IKE, "IKE_SA rekey collision won, waiting for delete for "
			 "redundant IKE_SA %s[%d]", other->new_sa->get_name(other->new_sa),
			 other->new_sa->get_unique_id(other->new_sa));
		other->new_sa->set_state(other->new_sa, IKE_REKEYED);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, other->new_sa);
		other->new_sa = NULL;
		charon->bus->set_sa(charon->bus, this->ike_sa);
	}

	establish_new(this);

	/* rekeying successful, delete the IKE_SA using a subtask */
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
	return this->collision &&
		   this->collision->get_type(this->collision) == TASK_IKE_REKEY;
}

METHOD(ike_rekey_t, collide, void,
	private_ike_rekey_t* this, task_t *other)
{
	DBG1(DBG_IKE, "detected %N collision with %N", task_type_names,
		 TASK_IKE_REKEY, task_type_names, other->get_type(other));

	switch (other->get_type(other))
	{
		case TASK_IKE_DELETE:
			conclude_undetected_collision(this);
			other->destroy(other);
			return;
		case TASK_IKE_REKEY:
		{
			private_ike_rekey_t *rekey = (private_ike_rekey_t*)other;

			if (!rekey->ike_init)
			{
				DBG1(DBG_IKE, "colliding exchange did not result in an IKE_SA, "
					 "ignore");
				other->destroy(other);
				return;
			}
			break;
		}
		default:
			break;
	}
	DESTROY_IF(this->collision);
	this->collision = other;
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
	DESTROY_IF(this->collision);
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

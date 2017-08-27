/*
 * Copyright (C) 2005-2007 Martin Willi
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

#include "child_rekey.h"

#include <daemon.h>
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
	u_int32_t spi;

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
	 * colliding task, may be delete or rekey
	 */
	task_t *collision;

	/**
	 * Indicate that peer destroyed the redundant child from collision.
	 * This happens if a peer's delete notification for the redundant
	 * child gets processed before the rekey job. If so, we must not
	 * touch the child created in the collision since it points to
	 * memory already freed.
	 */
	bool other_child_destroyed;
};

/**
 * Schedule a retry if rekeying temporary failed
 */
static void schedule_delayed_rekey(private_child_rekey_t *this)
{
	u_int32_t retry;
	job_t *job;

	retry = RETRY_INTERVAL - (random() % RETRY_JITTER);
	job = (job_t*)rekey_child_sa_job_create(
						this->child_sa->get_reqid(this->child_sa),
						this->child_sa->get_protocol(this->child_sa),
						this->child_sa->get_spi(this->child_sa, TRUE));
	DBG1(DBG_IKE, "CHILD_SA rekeying failed, trying again in %d seconds", retry);
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
	lib->scheduler->schedule_job(lib->scheduler, job, retry);
}

/**
 * Implementation of task_t.build for initiator, after rekeying
 */
static status_t build_i_delete(private_child_rekey_t *this, message_t *message)
{
	/* update exchange type to INFORMATIONAL for the delete */
	message->set_exchange_type(message, INFORMATIONAL);

	return this->child_delete->task.build(&this->child_delete->task, message);
}

/**
 * Implementation of task_t.process for initiator, after rekeying
 */
static status_t process_i_delete(private_child_rekey_t *this, message_t *message)
{
	return this->child_delete->task.process(&this->child_delete->task, message);
}

/**
 * find a child using the REKEY_SA notify
 */
static void find_child(private_child_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;
	protocol_id_t protocol;
	u_int32_t spi;

	notify = message->get_notify(message, REKEY_SA);
	if (notify)
	{
		protocol = notify->get_protocol_id(notify);
		spi = notify->get_spi(notify);

		if (protocol == PROTO_ESP || protocol == PROTO_AH)
		{
			this->child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol,
														spi, FALSE);
		}
	}
}

METHOD(task_t, build_i, status_t,
	private_child_rekey_t *this, message_t *message)
{
	notify_payload_t *notify;
	u_int32_t reqid;
	child_cfg_t *config;

	this->child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
												this->spi, TRUE);
	if (!this->child_sa)
	{	/* check if it is an outbound CHILD_SA */
		this->child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
													this->spi, FALSE);
		if (!this->child_sa)
		{	/* CHILD_SA is gone, unable to rekey. As an empty CREATE_CHILD_SA
			 * exchange is invalid, we fall back to an INFORMATIONAL exchange.*/
			message->set_exchange_type(message, INFORMATIONAL);
			return SUCCESS;
		}
		/* we work only with the inbound SPI */
		this->spi = this->child_sa->get_spi(this->child_sa, TRUE);
	}
	config = this->child_sa->get_config(this->child_sa);

	/* we just need the rekey notify ... */
	notify = notify_payload_create_from_protocol_and_type(PLV2_NOTIFY,
													this->protocol, REKEY_SA);
	notify->set_spi(notify, this->spi);
	message->add_payload(message, (payload_t*)notify);

	/* ... our CHILD_CREATE task does the hard work for us. */
	if (!this->child_create)
	{
		this->child_create = child_create_create(this->ike_sa,
									config->get_ref(config), TRUE, NULL, NULL);
	}
	reqid = this->child_sa->get_reqid(this->child_sa);
	this->child_create->use_reqid(this->child_create, reqid);

	if (this->child_create->task.build(&this->child_create->task,
									   message) != NEED_MORE)
	{
		schedule_delayed_rekey(this);
		return FAILED;
	}
	this->child_sa->set_state(this->child_sa, CHILD_REKEYING);

	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_child_rekey_t *this, message_t *message)
{
	/* let the CHILD_CREATE task process the message */
	this->child_create->task.process(&this->child_create->task, message);

	find_child(this, message);

	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_child_rekey_t *this, message_t *message)
{
	child_cfg_t *config;
	u_int32_t reqid;

	if (this->child_sa == NULL ||
		this->child_sa->get_state(this->child_sa) == CHILD_DELETING)
	{
		DBG1(DBG_IKE, "unable to rekey, CHILD_SA not found");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return SUCCESS;
	}

	/* let the CHILD_CREATE task build the response */
	reqid = this->child_sa->get_reqid(this->child_sa);
	this->child_create->use_reqid(this->child_create, reqid);
	config = this->child_sa->get_config(this->child_sa);
	this->child_create->set_config(this->child_create, config->get_ref(config));
	this->child_create->task.build(&this->child_create->task, message);

	if (message->get_payload(message, PLV2_SECURITY_ASSOCIATION) == NULL)
	{
		/* rekeying failed, reuse old child */
		this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
		return SUCCESS;
	}

	this->child_sa->set_state(this->child_sa, CHILD_REKEYING);

	/* invoke rekey hook */
	charon->bus->child_rekey(charon->bus, this->child_sa,
							 this->child_create->get_child(this->child_create));
	return SUCCESS;
}

/**
 * Handle a rekey collision
 */
static child_sa_t *handle_collision(private_child_rekey_t *this)
{
	child_sa_t *to_delete;

	if (this->collision->get_type(this->collision) == TASK_CHILD_REKEY)
	{
		chunk_t this_nonce, other_nonce;
		private_child_rekey_t *other = (private_child_rekey_t*)this->collision;

		this_nonce = this->child_create->get_lower_nonce(this->child_create);
		other_nonce = other->child_create->get_lower_nonce(other->child_create);

		/* if we have the lower nonce, delete rekeyed SA. If not, delete
		 * the redundant. */
		if (memcmp(this_nonce.ptr, other_nonce.ptr,
				   min(this_nonce.len, other_nonce.len)) > 0)
		{
			child_sa_t *child_sa;

			DBG1(DBG_IKE, "CHILD_SA rekey collision won, deleting old child");
			to_delete = this->child_sa;
			/* don't touch child other created, it has already been deleted */
			if (!this->other_child_destroyed)
			{
				/* disable close action for the redundand child */
				child_sa = other->child_create->get_child(other->child_create);
				if (child_sa)
				{
					child_sa->set_close_action(child_sa, ACTION_NONE);
				}
			}
		}
		else
		{
			DBG1(DBG_IKE, "CHILD_SA rekey collision lost, "
				 "deleting rekeyed child");
			to_delete = this->child_create->get_child(this->child_create);
		}
	}
	else
	{	/* CHILD_DELETE */
		child_delete_t *del = (child_delete_t*)this->collision;

		/* we didn't had a chance to compare the nonces, so we delete
		 * the CHILD_SA the other is not deleting. */
		if (del->get_child(del) != this->child_sa)
		{
			DBG1(DBG_IKE, "CHILD_SA rekey/delete collision, "
				 "deleting rekeyed child");
			to_delete = this->child_sa;
		}
		else
		{
			DBG1(DBG_IKE, "CHILD_SA rekey/delete collision, "
				 "deleting redundant child");
			to_delete = this->child_create->get_child(this->child_create);
		}
	}
	return to_delete;
}

METHOD(task_t, process_i, status_t,
	private_child_rekey_t *this, message_t *message)
{
	protocol_id_t protocol;
	u_int32_t spi;
	child_sa_t *to_delete;

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

	if (this->child_create->task.process(&this->child_create->task,
										 message) == NEED_MORE)
	{
		/* bad DH group while rekeying, try again */
		this->child_create->task.migrate(&this->child_create->task, this->ike_sa);
		return NEED_MORE;
	}
	if (message->get_payload(message, PLV2_SECURITY_ASSOCIATION) == NULL)
	{
		/* establishing new child failed, reuse old. but not when we
		 * received a delete in the meantime */
		if (!(this->collision &&
			  this->collision->get_type(this->collision) == TASK_CHILD_DELETE))
		{
			schedule_delayed_rekey(this);
		}
		return SUCCESS;
	}

	/* check for rekey collisions */
	if (this->collision)
	{
		to_delete = handle_collision(this);
	}
	else
	{
		to_delete = this->child_sa;
	}

	if (to_delete != this->child_create->get_child(this->child_create))
	{	/* invoke rekey hook if rekeying successful */
		charon->bus->child_rekey(charon->bus, this->child_sa,
							this->child_create->get_child(this->child_create));
	}

	if (to_delete == NULL)
	{
		return SUCCESS;
	}
	spi = to_delete->get_spi(to_delete, TRUE);
	protocol = to_delete->get_protocol(to_delete);

	/* rekeying done, delete the obsolete CHILD_SA using a subtask */
	this->child_delete = child_delete_create(this->ike_sa, protocol, spi, FALSE);
	this->public.task.build = (status_t(*)(task_t*,message_t*))build_i_delete;
	this->public.task.process = (status_t(*)(task_t*,message_t*))process_i_delete;

	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_child_rekey_t *this)
{
	return TASK_CHILD_REKEY;
}

METHOD(child_rekey_t, collide, void,
	private_child_rekey_t *this, task_t *other)
{
	/* the task manager only detects exchange collision, but not if
	 * the collision is for the same child. we check it here. */
	if (other->get_type(other) == TASK_CHILD_REKEY)
	{
		private_child_rekey_t *rekey = (private_child_rekey_t*)other;
		if (rekey->child_sa != this->child_sa)
		{
			/* not the same child => no collision */
			other->destroy(other);
			return;
		}
	}
	else if (other->get_type(other) == TASK_CHILD_DELETE)
	{
		child_delete_t *del = (child_delete_t*)other;
		if (this->collision &&
			this->collision->get_type(this->collision) == TASK_CHILD_REKEY)
		{
			private_child_rekey_t *rekey;

			rekey = (private_child_rekey_t*)this->collision;
			if (del->get_child(del) == rekey->child_create->get_child(rekey->child_create))
			{
				/* peer deletes redundant child created in collision */
				this->other_child_destroyed = TRUE;
				other->destroy(other);
				return;
			}
		}
		if (del->get_child(del) != this->child_sa)
		{
			/* not the same child => no collision */
			other->destroy(other);
			return;
		}
	}
	else
	{
		/* any other task is not critical for collisisions, ignore */
		other->destroy(other);
		return;
	}
	DBG1(DBG_IKE, "detected %N collision with %N", task_type_names,
		 TASK_CHILD_REKEY, task_type_names, other->get_type(other));
	DESTROY_IF(this->collision);
	this->collision = other;
}

METHOD(task_t, migrate, void,
	private_child_rekey_t *this, ike_sa_t *ike_sa)
{
	if (this->child_create)
	{
		this->child_create->task.migrate(&this->child_create->task, ike_sa);
	}
	if (this->child_delete)
	{
		this->child_delete->task.migrate(&this->child_delete->task, ike_sa);
	}
	DESTROY_IF(this->collision);

	this->ike_sa = ike_sa;
	this->collision = NULL;
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
	DESTROY_IF(this->collision);
	free(this);
}

/*
 * Described in header.
 */
child_rekey_t *child_rekey_create(ike_sa_t *ike_sa, protocol_id_t protocol,
								  u_int32_t spi)
{
	private_child_rekey_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
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
		this->child_create = child_create_create(ike_sa, NULL, TRUE, NULL, NULL);
	}

	return &this->public;
}

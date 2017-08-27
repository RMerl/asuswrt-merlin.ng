/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "informational.h"

#include <daemon.h>
#include <sa/ikev1/tasks/isakmp_delete.h>
#include <sa/ikev1/tasks/quick_delete.h>

#include <encoding/payloads/delete_payload.h>

typedef struct private_informational_t private_informational_t;

/**
 * Private members of a informational_t task.
 */
struct private_informational_t {

	/**
	 * Public methods and task_t interface.
	 */
	informational_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Notify payload to send
	 */
	notify_payload_t *notify;

	/**
	 * Delete subtask
	 */
	task_t *del;
};

/**
 * Cancel active quick mode after receiving an error
 */
static void cancel_quick_mode(private_informational_t *this)
{
	enumerator_t *enumerator;
	task_t *task;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_ACTIVE);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_QUICK_MODE)
		{
			this->ike_sa->flush_queue(this->ike_sa, TASK_QUEUE_ACTIVE);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_informational_t *this, message_t *message)
{
	message->add_payload(message, &this->notify->payload_interface);
	this->notify = NULL;
	return SUCCESS;
}

METHOD(task_t, process_r, status_t,
	private_informational_t *this, message_t *message)
{
	enumerator_t *enumerator;
	delete_payload_t *delete;
	notify_payload_t *notify;
	notify_type_t type;
	payload_t *payload;
	status_t status = SUCCESS;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		switch (payload->get_type(payload))
		{
			case PLV1_NOTIFY:
				notify = (notify_payload_t*)payload;
				type = notify->get_notify_type(notify);

				if (type == INITIAL_CONTACT_IKEV1)
				{
					this->ike_sa->set_condition(this->ike_sa,
												COND_INIT_CONTACT_SEEN, TRUE);
				}
				else if (type == UNITY_LOAD_BALANCE)
				{
					host_t *redirect, *me;
					chunk_t data;

					data = notify->get_notification_data(notify);
					redirect = host_create_from_chunk(AF_INET, data,
													  IKEV2_UDP_PORT);
					if (redirect)
					{	/* treat the redirect as reauthentication */
						DBG1(DBG_IKE, "received %N notify, redirected to %H",
							 notify_type_names, type, redirect);
						/* Cisco boxes reject the first message from 4500 */
						me = this->ike_sa->get_my_host(this->ike_sa);
						me->set_port(me, charon->socket->get_port(
														charon->socket, FALSE));
						this->ike_sa->set_other_host(this->ike_sa, redirect);
						status = this->ike_sa->reauth(this->ike_sa);
						enumerator->destroy(enumerator);
						return status;
					}
					else
					{
						DBG1(DBG_IKE, "received %N notify, invalid address");
					}
				}
				else if (type < 16384)
				{
					DBG1(DBG_IKE, "received %N error notify",
						 notify_type_names, type);
					if (this->ike_sa->get_state(this->ike_sa) == IKE_CONNECTING)
					{	/* only critical during main mode */
						status = FAILED;
					}
					switch (type)
					{
						case INVALID_ID_INFORMATION:
						case NO_PROPOSAL_CHOSEN:
							cancel_quick_mode(this);
							break;
						default:
							break;
					}
					break;
				}
				else
				{
					DBG1(DBG_IKE, "received %N notify",
						 notify_type_names, type);
				}
				continue;
			case PLV1_DELETE:
				if (!this->del)
				{
					delete = (delete_payload_t*)payload;
					if (delete->get_protocol_id(delete) == PROTO_IKE)
					{
						this->del = (task_t*)isakmp_delete_create(this->ike_sa,
																  FALSE);
					}
					else
					{
						this->del = (task_t*)quick_delete_create(this->ike_sa,
												PROTO_NONE, 0, FALSE, FALSE);
					}
				}
				break;
			default:
				continue;
		}
		break;
	}
	enumerator->destroy(enumerator);

	if (this->del && status == SUCCESS)
	{
		return this->del->process(this->del, message);
	}
	return status;
}

METHOD(task_t, build_r, status_t,
	private_informational_t *this, message_t *message)
{
	if (this->del)
	{
		return this->del->build(this->del, message);
	}
	return FAILED;
}

METHOD(task_t, process_i, status_t,
	private_informational_t *this, message_t *message)
{
	return FAILED;
}

METHOD(task_t, get_type, task_type_t,
	private_informational_t *this)
{
	return TASK_INFORMATIONAL;
}

METHOD(task_t, migrate, void,
	private_informational_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_informational_t *this)
{
	DESTROY_IF(this->notify);
	DESTROY_IF(this->del);
	free(this);
}

/*
 * Described in header.
 */
informational_t *informational_create(ike_sa_t *ike_sa, notify_payload_t *notify)
{
	private_informational_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.notify = notify,
	);

	if (notify)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}

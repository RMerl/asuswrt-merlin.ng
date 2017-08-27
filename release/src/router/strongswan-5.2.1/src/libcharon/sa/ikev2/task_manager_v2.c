/*
 * Copyright (C) 2007-2014 Tobias Brunner
 * Copyright (C) 2007-2010 Martin Willi
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

#include "task_manager_v2.h"

#include <math.h>

#include <collections/array.h>
#include <daemon.h>
#include <sa/ikev2/tasks/ike_init.h>
#include <sa/ikev2/tasks/ike_natd.h>
#include <sa/ikev2/tasks/ike_mobike.h>
#include <sa/ikev2/tasks/ike_auth.h>
#include <sa/ikev2/tasks/ike_auth_lifetime.h>
#include <sa/ikev2/tasks/ike_cert_pre.h>
#include <sa/ikev2/tasks/ike_cert_post.h>
#include <sa/ikev2/tasks/ike_rekey.h>
#include <sa/ikev2/tasks/ike_reauth.h>
#include <sa/ikev2/tasks/ike_delete.h>
#include <sa/ikev2/tasks/ike_config.h>
#include <sa/ikev2/tasks/ike_dpd.h>
#include <sa/ikev2/tasks/ike_vendor.h>
#include <sa/ikev2/tasks/child_create.h>
#include <sa/ikev2/tasks/child_rekey.h>
#include <sa/ikev2/tasks/child_delete.h>
#include <encoding/payloads/delete_payload.h>
#include <encoding/payloads/unknown_payload.h>
#include <processing/jobs/retransmit_job.h>
#include <processing/jobs/delete_ike_sa_job.h>

#ifdef ME
#include <sa/ikev2/tasks/ike_me.h>
#endif

typedef struct exchange_t exchange_t;

/**
 * An exchange in the air, used do detect and handle retransmission
 */
struct exchange_t {

	/**
	 * Message ID used for this transaction
	 */
	u_int32_t mid;

	/**
	 * generated packet for retransmission
	 */
	packet_t *packet;
};

typedef struct private_task_manager_t private_task_manager_t;

/**
 * private data of the task manager
 */
struct private_task_manager_t {

	/**
	 * public functions
	 */
	task_manager_v2_t public;

	/**
	 * associated IKE_SA we are serving
	 */
	ike_sa_t *ike_sa;

	/**
	 * Exchange we are currently handling as responder
	 */
	struct {
		/**
		 * Message ID of the exchange
		 */
		u_int32_t mid;

		/**
		 * packet(s) for retransmission
		 */
		array_t *packets;

		/**
		 * Helper to defragment the request
		 */
		message_t *defrag;

	} responding;

	/**
	 * Exchange we are currently handling as initiator
	 */
	struct {
		/**
		 * Message ID of the exchange
		 */
		u_int32_t mid;

		/**
		 * how many times we have retransmitted so far
		 */
		u_int retransmitted;

		/**
		 * packet(s) for retransmission
		 */
		array_t *packets;

		/**
		 * type of the initated exchange
		 */
		exchange_type_t type;

		/**
		 * TRUE if exchange was deferred because no path was available
		 */
		bool deferred;

		/**
		 * Helper to defragment the response
		 */
		message_t *defrag;

	} initiating;

	/**
	 * Array of queued tasks not yet in action
	 */
	array_t *queued_tasks;

	/**
	 * Array of active tasks, initiated by ourselve
	 */
	array_t *active_tasks;

	/**
	 * Array of tasks initiated by peer
	 */
	array_t *passive_tasks;

	/**
	 * the task manager has been reset
	 */
	bool reset;

	/**
	 * Number of times we retransmit messages before giving up
	 */
	u_int retransmit_tries;

	/**
	 * Retransmission timeout
	 */
	double retransmit_timeout;

	/**
	 * Base to calculate retransmission timeout
	 */
	double retransmit_base;
};

/**
 * Reset retransmission packet list
 */
static void clear_packets(array_t *array)
{
	packet_t *packet;

	while (array_remove(array, ARRAY_TAIL, &packet))
	{
		packet->destroy(packet);
	}
}

METHOD(task_manager_t, flush_queue, void,
	private_task_manager_t *this, task_queue_t queue)
{
	array_t *array;
	task_t *task;

	switch (queue)
	{
		case TASK_QUEUE_ACTIVE:
			array = this->active_tasks;
			break;
		case TASK_QUEUE_PASSIVE:
			array = this->passive_tasks;
			break;
		case TASK_QUEUE_QUEUED:
			array = this->queued_tasks;
			break;
		default:
			return;
	}
	while (array_remove(array, ARRAY_TAIL, &task))
	{
		task->destroy(task);
	}
}

METHOD(task_manager_t, flush, void,
	private_task_manager_t *this)
{
	flush_queue(this, TASK_QUEUE_QUEUED);
	flush_queue(this, TASK_QUEUE_PASSIVE);
	flush_queue(this, TASK_QUEUE_ACTIVE);
}

/**
 * move a task of a specific type from the queue to the active list
 */
static bool activate_task(private_task_manager_t *this, task_type_t type)
{
	enumerator_t *enumerator;
	task_t *task;
	bool found = FALSE;

	enumerator = array_create_enumerator(this->queued_tasks);
	while (enumerator->enumerate(enumerator, (void**)&task))
	{
		if (task->get_type(task) == type)
		{
			DBG2(DBG_IKE, "  activating %N task", task_type_names, type);
			array_remove_at(this->queued_tasks, enumerator);
			array_insert(this->active_tasks, ARRAY_TAIL, task);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Send packets in the given array (they get cloned). Optionally, the
 * source and destination addresses are changed before sending it.
 */
static void send_packets(private_task_manager_t *this, array_t *packets,
						 host_t *src, host_t *dst)
{
	packet_t *packet, *clone;
	int i;

	for (i = 0; i < array_count(packets); i++)
	{
		array_get(packets, i, &packet);
		clone = packet->clone(packet);
		if (src)
		{
			clone->set_source(clone, src->clone(src));
		}
		if (dst)
		{
			clone->set_destination(clone, dst->clone(dst));
		}
		charon->sender->send(charon->sender, clone);
	}
}

/**
 * Generates the given message and stores packet(s) in the given array
 */
static bool generate_message(private_task_manager_t *this, message_t *message,
							 array_t **packets)
{
	enumerator_t *fragments;
	packet_t *fragment;

	if (this->ike_sa->generate_message_fragmented(this->ike_sa, message,
												  &fragments) != SUCCESS)
	{
		return FALSE;
	}
	while (fragments->enumerate(fragments, &fragment))
	{
		array_insert_create(packets, ARRAY_TAIL, fragment);
	}
	fragments->destroy(fragments);
	array_compress(*packets);
	return TRUE;
}

METHOD(task_manager_t, retransmit, status_t,
	private_task_manager_t *this, u_int32_t message_id)
{
	if (message_id == this->initiating.mid &&
		array_count(this->initiating.packets))
	{
		u_int32_t timeout;
		job_t *job;
		enumerator_t *enumerator;
		packet_t *packet;
		task_t *task;
		ike_mobike_t *mobike = NULL;

		array_get(this->initiating.packets, 0, &packet);

		/* check if we are retransmitting a MOBIKE routability check */
		if (this->initiating.type == INFORMATIONAL)
		{
			enumerator = array_create_enumerator(this->active_tasks);
			while (enumerator->enumerate(enumerator, (void*)&task))
			{
				if (task->get_type(task) == TASK_IKE_MOBIKE)
				{
					mobike = (ike_mobike_t*)task;
					break;
				}
			}
			enumerator->destroy(enumerator);
		}

		if (!mobike || !mobike->is_probing(mobike))
		{
			if (this->initiating.retransmitted <= this->retransmit_tries)
			{
				timeout = (u_int32_t)(this->retransmit_timeout * 1000.0 *
					pow(this->retransmit_base, this->initiating.retransmitted));
			}
			else
			{
				DBG1(DBG_IKE, "giving up after %d retransmits",
					 this->initiating.retransmitted - 1);
				charon->bus->alert(charon->bus, ALERT_RETRANSMIT_SEND_TIMEOUT,
								   packet);
				return DESTROY_ME;
			}

			if (this->initiating.retransmitted)
			{
				DBG1(DBG_IKE, "retransmit %d of request with message ID %d",
					 this->initiating.retransmitted, message_id);
				charon->bus->alert(charon->bus, ALERT_RETRANSMIT_SEND, packet);
			}
			if (!mobike)
			{
				send_packets(this, this->initiating.packets,
							 this->ike_sa->get_my_host(this->ike_sa),
							 this->ike_sa->get_other_host(this->ike_sa));
			}
			else
			{
				if (!mobike->transmit(mobike, packet))
				{
					DBG1(DBG_IKE, "no route found to reach peer, MOBIKE update "
						 "deferred");
					this->ike_sa->set_condition(this->ike_sa, COND_STALE, TRUE);
					this->initiating.deferred = TRUE;
					return SUCCESS;
				}
				else if (mobike->is_probing(mobike))
				{
					timeout = ROUTEABILITY_CHECK_INTERVAL;
				}
			}
		}
		else
		{	/* for routeability checks, we use a more aggressive behavior */
			if (this->initiating.retransmitted <= ROUTEABILITY_CHECK_TRIES)
			{
				timeout = ROUTEABILITY_CHECK_INTERVAL;
			}
			else
			{
				DBG1(DBG_IKE, "giving up after %d path probings",
					 this->initiating.retransmitted - 1);
				return DESTROY_ME;
			}

			if (this->initiating.retransmitted)
			{
				DBG1(DBG_IKE, "path probing attempt %d",
					 this->initiating.retransmitted);
			}
			/* TODO-FRAG: presumably these small packets are not fragmented,
			 * we should maybe ensure this is the case when generating them */
			if (!mobike->transmit(mobike, packet))
			{
				DBG1(DBG_IKE, "no route found to reach peer, path probing "
					 "deferred");
				this->ike_sa->set_condition(this->ike_sa, COND_STALE, TRUE);
				this->initiating.deferred = TRUE;
				return SUCCESS;
			}
		}

		this->initiating.retransmitted++;
		job = (job_t*)retransmit_job_create(this->initiating.mid,
											this->ike_sa->get_id(this->ike_sa));
		lib->scheduler->schedule_job_ms(lib->scheduler, job, timeout);
	}
	return SUCCESS;
}

METHOD(task_manager_t, initiate, status_t,
	private_task_manager_t *this)
{
	enumerator_t *enumerator;
	task_t *task;
	message_t *message;
	host_t *me, *other;
	exchange_type_t exchange = 0;

	if (this->initiating.type != EXCHANGE_TYPE_UNDEFINED)
	{
		DBG2(DBG_IKE, "delaying task initiation, %N exchange in progress",
				exchange_type_names, this->initiating.type);
		/* do not initiate if we already have a message in the air */
		if (this->initiating.deferred)
		{	/* re-initiate deferred exchange */
			this->initiating.deferred = FALSE;
			this->initiating.retransmitted = 0;
			return retransmit(this, this->initiating.mid);
		}
		return SUCCESS;
	}

	if (array_count(this->active_tasks) == 0)
	{
		DBG2(DBG_IKE, "activating new tasks");
		switch (this->ike_sa->get_state(this->ike_sa))
		{
			case IKE_CREATED:
				activate_task(this, TASK_IKE_VENDOR);
				if (activate_task(this, TASK_IKE_INIT))
				{
					this->initiating.mid = 0;
					exchange = IKE_SA_INIT;
					activate_task(this, TASK_IKE_NATD);
					activate_task(this, TASK_IKE_CERT_PRE);
#ifdef ME
					/* this task has to be activated before the TASK_IKE_AUTH
					 * task, because that task pregenerates the packet after
					 * which no payloads can be added to the message anymore.
					 */
					activate_task(this, TASK_IKE_ME);
#endif /* ME */
					activate_task(this, TASK_IKE_AUTH);
					activate_task(this, TASK_IKE_CERT_POST);
					activate_task(this, TASK_IKE_CONFIG);
					activate_task(this, TASK_CHILD_CREATE);
					activate_task(this, TASK_IKE_AUTH_LIFETIME);
					activate_task(this, TASK_IKE_MOBIKE);
				}
				break;
			case IKE_ESTABLISHED:
				if (activate_task(this, TASK_IKE_MOBIKE))
				{
					exchange = INFORMATIONAL;
					break;
				}
				if (activate_task(this, TASK_IKE_DELETE))
				{
					exchange = INFORMATIONAL;
					break;
				}
				if (activate_task(this, TASK_CHILD_DELETE))
				{
					exchange = INFORMATIONAL;
					break;
				}
				if (activate_task(this, TASK_IKE_REAUTH))
				{
					exchange = INFORMATIONAL;
					break;
				}
				if (activate_task(this, TASK_CHILD_CREATE))
				{
					exchange = CREATE_CHILD_SA;
					break;
				}
				if (activate_task(this, TASK_CHILD_REKEY))
				{
					exchange = CREATE_CHILD_SA;
					break;
				}
				if (activate_task(this, TASK_IKE_REKEY))
				{
					exchange = CREATE_CHILD_SA;
					break;
				}
				if (activate_task(this, TASK_IKE_DPD))
				{
					exchange = INFORMATIONAL;
					break;
				}
				if (activate_task(this, TASK_IKE_AUTH_LIFETIME))
				{
					exchange = INFORMATIONAL;
					break;
				}
#ifdef ME
				if (activate_task(this, TASK_IKE_ME))
				{
					exchange = ME_CONNECT;
					break;
				}
#endif /* ME */
			case IKE_REKEYING:
				if (activate_task(this, TASK_IKE_DELETE))
				{
					exchange = INFORMATIONAL;
					break;
				}
			case IKE_DELETING:
			default:
				break;
		}
	}
	else
	{
		DBG2(DBG_IKE, "reinitiating already active tasks");
		enumerator = array_create_enumerator(this->active_tasks);
		while (enumerator->enumerate(enumerator, &task))
		{
			DBG2(DBG_IKE, "  %N task", task_type_names, task->get_type(task));
			switch (task->get_type(task))
			{
				case TASK_IKE_INIT:
					exchange = IKE_SA_INIT;
					break;
				case TASK_IKE_AUTH:
					exchange = IKE_AUTH;
					break;
				case TASK_CHILD_CREATE:
				case TASK_CHILD_REKEY:
				case TASK_IKE_REKEY:
					exchange = CREATE_CHILD_SA;
					break;
				case TASK_IKE_MOBIKE:
					exchange = INFORMATIONAL;
					break;
				default:
					continue;
			}
			break;
		}
		enumerator->destroy(enumerator);
	}

	if (exchange == 0)
	{
		DBG2(DBG_IKE, "nothing to initiate");
		/* nothing to do yet... */
		return SUCCESS;
	}

	me = this->ike_sa->get_my_host(this->ike_sa);
	other = this->ike_sa->get_other_host(this->ike_sa);

	message = message_create(IKEV2_MAJOR_VERSION, IKEV2_MINOR_VERSION);
	message->set_message_id(message, this->initiating.mid);
	message->set_source(message, me->clone(me));
	message->set_destination(message, other->clone(other));
	message->set_exchange_type(message, exchange);
	this->initiating.type = exchange;
	this->initiating.retransmitted = 0;
	this->initiating.deferred = FALSE;

	enumerator = array_create_enumerator(this->active_tasks);
	while (enumerator->enumerate(enumerator, &task))
	{
		switch (task->build(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				array_remove_at(this->active_tasks, enumerator);
				task->destroy(task);
				break;
			case NEED_MORE:
				/* processed, but task needs another exchange */
				break;
			case FAILED:
			default:
				this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
				if (this->ike_sa->get_state(this->ike_sa) != IKE_CONNECTING)
				{
					charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				}
				/* FALL */
			case DESTROY_ME:
				/* critical failure, destroy IKE_SA */
				enumerator->destroy(enumerator);
				message->destroy(message);
				flush(this);
				return DESTROY_ME;
		}
	}
	enumerator->destroy(enumerator);

	/* update exchange type if a task changed it */
	this->initiating.type = message->get_exchange_type(message);

	if (!generate_message(this, message, &this->initiating.packets))
	{
		/* message generation failed. There is nothing more to do than to
		 * close the SA */
		message->destroy(message);
		flush(this);
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}
	message->destroy(message);

	array_compress(this->active_tasks);
	array_compress(this->queued_tasks);

	return retransmit(this, this->initiating.mid);
}

/**
 * handle an incoming response message
 */
static status_t process_response(private_task_manager_t *this,
								 message_t *message)
{
	enumerator_t *enumerator;
	task_t *task;

	if (message->get_exchange_type(message) != this->initiating.type)
	{
		DBG1(DBG_IKE, "received %N response, but expected %N",
			 exchange_type_names, message->get_exchange_type(message),
			 exchange_type_names, this->initiating.type);
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}

	/* catch if we get resetted while processing */
	this->reset = FALSE;
	enumerator = array_create_enumerator(this->active_tasks);
	while (enumerator->enumerate(enumerator, &task))
	{
		switch (task->process(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				array_remove_at(this->active_tasks, enumerator);
				task->destroy(task);
				break;
			case NEED_MORE:
				/* processed, but task needs another exchange */
				break;
			case FAILED:
			default:
				charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				/* FALL */
			case DESTROY_ME:
				/* critical failure, destroy IKE_SA */
				array_remove_at(this->active_tasks, enumerator);
				enumerator->destroy(enumerator);
				task->destroy(task);
				return DESTROY_ME;
		}
		if (this->reset)
		{	/* start all over again if we were reset */
			this->reset = FALSE;
			enumerator->destroy(enumerator);
			return initiate(this);
		}
	}
	enumerator->destroy(enumerator);

	this->initiating.mid++;
	this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
	clear_packets(this->initiating.packets);

	array_compress(this->active_tasks);

	return initiate(this);
}

/**
 * handle exchange collisions
 */
static bool handle_collisions(private_task_manager_t *this, task_t *task)
{
	enumerator_t *enumerator;
	task_t *active;
	task_type_t type;

	type = task->get_type(task);

	/* do we have to check  */
	if (type == TASK_IKE_REKEY || type == TASK_CHILD_REKEY ||
		type == TASK_CHILD_DELETE || type == TASK_IKE_DELETE ||
		type == TASK_IKE_REAUTH)
	{
		/* find an exchange collision, and notify these tasks */
		enumerator = array_create_enumerator(this->active_tasks);
		while (enumerator->enumerate(enumerator, &active))
		{
			switch (active->get_type(active))
			{
				case TASK_IKE_REKEY:
					if (type == TASK_IKE_REKEY || type == TASK_IKE_DELETE ||
						type == TASK_IKE_REAUTH)
					{
						ike_rekey_t *rekey = (ike_rekey_t*)active;
						rekey->collide(rekey, task);
						break;
					}
					continue;
				case TASK_CHILD_REKEY:
					if (type == TASK_CHILD_REKEY || type == TASK_CHILD_DELETE)
					{
						child_rekey_t *rekey = (child_rekey_t*)active;
						rekey->collide(rekey, task);
						break;
					}
					continue;
				default:
					continue;
			}
			enumerator->destroy(enumerator);
			return TRUE;
		}
		enumerator->destroy(enumerator);
	}
	return FALSE;
}

/**
 * build a response depending on the "passive" task list
 */
static status_t build_response(private_task_manager_t *this, message_t *request)
{
	enumerator_t *enumerator;
	task_t *task;
	message_t *message;
	host_t *me, *other;
	bool delete = FALSE, hook = FALSE;
	ike_sa_id_t *id = NULL;
	u_int64_t responder_spi = 0;
	bool result;

	me = request->get_destination(request);
	other = request->get_source(request);

	message = message_create(IKEV2_MAJOR_VERSION, IKEV2_MINOR_VERSION);
	message->set_exchange_type(message, request->get_exchange_type(request));
	/* send response along the path the request came in */
	message->set_source(message, me->clone(me));
	message->set_destination(message, other->clone(other));
	message->set_message_id(message, this->responding.mid);
	message->set_request(message, FALSE);

	enumerator = array_create_enumerator(this->passive_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->build(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				array_remove_at(this->passive_tasks, enumerator);
				if (!handle_collisions(this, task))
				{
					task->destroy(task);
				}
				break;
			case NEED_MORE:
				/* processed, but task needs another exchange */
				if (handle_collisions(this, task))
				{
					array_remove_at(this->passive_tasks, enumerator);
				}
				break;
			case FAILED:
			default:
				hook = TRUE;
				/* FALL */
			case DESTROY_ME:
				/* destroy IKE_SA, but SEND response first */
				delete = TRUE;
				break;
		}
		if (delete)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* RFC 5996, section 2.6 mentions that in the event of a failure during
	 * IKE_SA_INIT the responder's SPI will be 0 in the response, while it
	 * actually explicitly allows it to be non-zero.  Since we use the responder
	 * SPI to create hashes in the IKE_SA manager we can only set the SPI to
	 * zero temporarily, otherwise checking the SA in would fail. */
	if (delete && request->get_exchange_type(request) == IKE_SA_INIT)
	{
		id = this->ike_sa->get_id(this->ike_sa);
		responder_spi = id->get_responder_spi(id);
		id->set_responder_spi(id, 0);
	}

	/* message complete, send it */
	clear_packets(this->responding.packets);
	result = generate_message(this, message, &this->responding.packets);
	message->destroy(message);
	if (id)
	{
		id->set_responder_spi(id, responder_spi);
	}
	if (!result)
	{
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}

	send_packets(this, this->responding.packets, NULL, NULL);
	if (delete)
	{
		if (hook)
		{
			charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		}
		return DESTROY_ME;
	}

	array_compress(this->passive_tasks);

	return SUCCESS;
}

/**
 * handle an incoming request message
 */
static status_t process_request(private_task_manager_t *this,
								message_t *message)
{
	enumerator_t *enumerator;
	task_t *task = NULL;
	payload_t *payload;
	notify_payload_t *notify;
	delete_payload_t *delete;

	if (array_count(this->passive_tasks) == 0)
	{	/* create tasks depending on request type, if not already some queued */
		switch (message->get_exchange_type(message))
		{
			case IKE_SA_INIT:
			{
				task = (task_t*)ike_vendor_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_init_create(this->ike_sa, FALSE, NULL);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_natd_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_cert_pre_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
#ifdef ME
				task = (task_t*)ike_me_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
#endif /* ME */
				task = (task_t*)ike_auth_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_cert_post_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_config_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)child_create_create(this->ike_sa, NULL, FALSE,
													NULL, NULL);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_auth_lifetime_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				task = (task_t*)ike_mobike_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				break;
			}
			case CREATE_CHILD_SA:
			{	/* FIXME: we should prevent this on mediation connections */
				bool notify_found = FALSE, ts_found = FALSE;

				if (this->ike_sa->get_state(this->ike_sa) == IKE_CREATED ||
					this->ike_sa->get_state(this->ike_sa) == IKE_CONNECTING)
				{
					DBG1(DBG_IKE, "received CREATE_CHILD_SA request for "
						 "unestablished IKE_SA, rejected");
					return FAILED;
				}

				enumerator = message->create_payload_enumerator(message);
				while (enumerator->enumerate(enumerator, &payload))
				{
					switch (payload->get_type(payload))
					{
						case PLV2_NOTIFY:
						{	/* if we find a rekey notify, its CHILD_SA rekeying */
							notify = (notify_payload_t*)payload;
							if (notify->get_notify_type(notify) == REKEY_SA &&
								(notify->get_protocol_id(notify) == PROTO_AH ||
								 notify->get_protocol_id(notify) == PROTO_ESP))
							{
								notify_found = TRUE;
							}
							break;
						}
						case PLV2_TS_INITIATOR:
						case PLV2_TS_RESPONDER:
						{	/* if we don't find a TS, its IKE rekeying */
							ts_found = TRUE;
							break;
						}
						default:
							break;
					}
				}
				enumerator->destroy(enumerator);

				if (ts_found)
				{
					if (notify_found)
					{
						task = (task_t*)child_rekey_create(this->ike_sa,
														   PROTO_NONE, 0);
					}
					else
					{
						task = (task_t*)child_create_create(this->ike_sa, NULL,
															FALSE, NULL, NULL);
					}
				}
				else
				{
					task = (task_t*)ike_rekey_create(this->ike_sa, FALSE);
				}
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				break;
			}
			case INFORMATIONAL:
			{
				enumerator = message->create_payload_enumerator(message);
				while (enumerator->enumerate(enumerator, &payload))
				{
					switch (payload->get_type(payload))
					{
						case PLV2_NOTIFY:
						{
							notify = (notify_payload_t*)payload;
							switch (notify->get_notify_type(notify))
							{
								case ADDITIONAL_IP4_ADDRESS:
								case ADDITIONAL_IP6_ADDRESS:
								case NO_ADDITIONAL_ADDRESSES:
								case UPDATE_SA_ADDRESSES:
								case NO_NATS_ALLOWED:
								case UNACCEPTABLE_ADDRESSES:
								case UNEXPECTED_NAT_DETECTED:
								case COOKIE2:
								case NAT_DETECTION_SOURCE_IP:
								case NAT_DETECTION_DESTINATION_IP:
									task = (task_t*)ike_mobike_create(
															this->ike_sa, FALSE);
									break;
								case AUTH_LIFETIME:
									task = (task_t*)ike_auth_lifetime_create(
															this->ike_sa, FALSE);
									break;
								case AUTHENTICATION_FAILED:
									/* initiator failed to authenticate us.
									 * We use ike_delete to handle this, which
									 * invokes all the required hooks. */
									task = (task_t*)ike_delete_create(
														this->ike_sa, FALSE);
								default:
									break;
							}
							break;
						}
						case PLV2_DELETE:
						{
							delete = (delete_payload_t*)payload;
							if (delete->get_protocol_id(delete) == PROTO_IKE)
							{
								task = (task_t*)ike_delete_create(this->ike_sa,
																FALSE);
							}
							else
							{
								task = (task_t*)child_delete_create(this->ike_sa,
														PROTO_NONE, 0, FALSE);
							}
							break;
						}
						default:
							break;
					}
					if (task)
					{
						break;
					}
				}
				enumerator->destroy(enumerator);

				if (task == NULL)
				{
					task = (task_t*)ike_dpd_create(FALSE);
				}
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
				break;
			}
#ifdef ME
			case ME_CONNECT:
			{
				task = (task_t*)ike_me_create(this->ike_sa, FALSE);
				array_insert(this->passive_tasks, ARRAY_TAIL, task);
			}
#endif /* ME */
			default:
				break;
		}
	}

	/* let the tasks process the message */
	enumerator = array_create_enumerator(this->passive_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->process(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				array_remove_at(this->passive_tasks, enumerator);
				task->destroy(task);
				break;
			case NEED_MORE:
				/* processed, but task needs at least another call to build() */
				break;
			case FAILED:
			default:
				charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				/* FALL */
			case DESTROY_ME:
				/* critical failure, destroy IKE_SA */
				array_remove_at(this->passive_tasks, enumerator);
				enumerator->destroy(enumerator);
				task->destroy(task);
				return DESTROY_ME;
		}
	}
	enumerator->destroy(enumerator);

	return build_response(this, message);
}

METHOD(task_manager_t, incr_mid, void,
	private_task_manager_t *this, bool initiate)
{
	if (initiate)
	{
		this->initiating.mid++;
	}
	else
	{
		this->responding.mid++;
	}
}

/**
 * Handle the given IKE fragment, if it is one.
 *
 * Returns SUCCESS if the message is not a fragment, and NEED_MORE if it was
 * handled properly.  Error states are  returned if the fragment was invalid or
 * the reassembled message could not have been processed properly.
 */
static status_t handle_fragment(private_task_manager_t *this,
								message_t **defrag, message_t *msg)
{
	message_t *reassembled;
	status_t status;

	if (!msg->get_payload(msg, PLV2_FRAGMENT))
	{
		return SUCCESS;
	}
	if (!*defrag)
	{
		*defrag = message_create_defrag(msg);
		if (!*defrag)
		{
			return FAILED;
		}
	}
	status = (*defrag)->add_fragment(*defrag, msg);
	if (status == SUCCESS)
	{
		/* reinject the reassembled message */
		reassembled = *defrag;
		*defrag = NULL;
		status = this->ike_sa->process_message(this->ike_sa, reassembled);
		if (status == SUCCESS)
		{
			/* avoid processing the last fragment */
			status = NEED_MORE;
		}
		reassembled->destroy(reassembled);
	}
	return status;
}

/**
 * Send a notify back to the sender
 */
static void send_notify_response(private_task_manager_t *this,
								 message_t *request, notify_type_t type,
								 chunk_t data)
{
	message_t *response;
	packet_t *packet;
	host_t *me, *other;

	response = message_create(IKEV2_MAJOR_VERSION, IKEV2_MINOR_VERSION);
	response->set_exchange_type(response, request->get_exchange_type(request));
	response->set_request(response, FALSE);
	response->set_message_id(response, request->get_message_id(request));
	response->add_notify(response, FALSE, type, data);
	me = this->ike_sa->get_my_host(this->ike_sa);
	if (me->is_anyaddr(me))
	{
		me = request->get_destination(request);
		this->ike_sa->set_my_host(this->ike_sa, me->clone(me));
	}
	other = this->ike_sa->get_other_host(this->ike_sa);
	if (other->is_anyaddr(other))
	{
		other = request->get_source(request);
		this->ike_sa->set_other_host(this->ike_sa, other->clone(other));
	}
	response->set_source(response, me->clone(me));
	response->set_destination(response, other->clone(other));
	if (this->ike_sa->generate_message(this->ike_sa, response,
									   &packet) == SUCCESS)
	{
		charon->sender->send(charon->sender, packet);
	}
	response->destroy(response);
}

/**
 * Parse the given message and verify that it is valid.
 */
static status_t parse_message(private_task_manager_t *this, message_t *msg)
{
	status_t status;
	u_int8_t type = 0;

	status = msg->parse_body(msg, this->ike_sa->get_keymat(this->ike_sa));

	if (status == SUCCESS)
	{	/* check for unsupported critical payloads */
		enumerator_t *enumerator;
		unknown_payload_t *unknown;
		payload_t *payload;

		enumerator = msg->create_payload_enumerator(msg);
		while (enumerator->enumerate(enumerator, &payload))
		{
			unknown = (unknown_payload_t*)payload;
			type = payload->get_type(payload);
			if (!payload_is_known(type) &&
				unknown->is_critical(unknown))
			{
				DBG1(DBG_ENC, "payload type %N is not supported, "
					 "but its critical!", payload_type_names, type);
				status = NOT_SUPPORTED;
				break;
			}
		}
		enumerator->destroy(enumerator);
	}

	if (status != SUCCESS)
	{
		bool is_request = msg->get_request(msg);

		switch (status)
		{
			case NOT_SUPPORTED:
				DBG1(DBG_IKE, "critical unknown payloads found");
				if (is_request)
				{
					send_notify_response(this, msg,
										 UNSUPPORTED_CRITICAL_PAYLOAD,
										 chunk_from_thing(type));
					incr_mid(this, FALSE);
				}
				break;
			case PARSE_ERROR:
				DBG1(DBG_IKE, "message parsing failed");
				if (is_request)
				{
					send_notify_response(this, msg,
										 INVALID_SYNTAX, chunk_empty);
					incr_mid(this, FALSE);
				}
				break;
			case VERIFY_ERROR:
				DBG1(DBG_IKE, "message verification failed");
				if (is_request)
				{
					send_notify_response(this, msg,
										 INVALID_SYNTAX, chunk_empty);
					incr_mid(this, FALSE);
				}
				break;
			case FAILED:
				DBG1(DBG_IKE, "integrity check failed");
				/* ignored */
				break;
			case INVALID_STATE:
				DBG1(DBG_IKE, "found encrypted message, but no keys available");
			default:
				break;
		}
		DBG1(DBG_IKE, "%N %s with message ID %d processing failed",
			 exchange_type_names, msg->get_exchange_type(msg),
			 is_request ? "request" : "response",
			 msg->get_message_id(msg));

		charon->bus->alert(charon->bus, ALERT_PARSE_ERROR_BODY, msg, status);

		if (this->ike_sa->get_state(this->ike_sa) == IKE_CREATED)
		{	/* invalid initiation attempt, close SA */
			return DESTROY_ME;
		}
	}
	return status;
}


METHOD(task_manager_t, process_message, status_t,
	private_task_manager_t *this, message_t *msg)
{
	host_t *me, *other;
	status_t status;
	u_int32_t mid;
	bool schedule_delete_job = FALSE;

	charon->bus->message(charon->bus, msg, TRUE, FALSE);
	status = parse_message(this, msg);
	if (status != SUCCESS)
	{
		return status;
	}

	me = msg->get_destination(msg);
	other = msg->get_source(msg);

	/* if this IKE_SA is virgin, we check for a config */
	if (this->ike_sa->get_ike_cfg(this->ike_sa) == NULL)
	{
		ike_cfg_t *ike_cfg;

		ike_cfg = charon->backends->get_ike_cfg(charon->backends,
												me, other, IKEV2);
		if (ike_cfg == NULL)
		{
			/* no config found for these hosts, destroy */
			DBG1(DBG_IKE, "no IKE config found for %H...%H, sending %N",
				 me, other, notify_type_names, NO_PROPOSAL_CHOSEN);
			send_notify_response(this, msg,
								 NO_PROPOSAL_CHOSEN, chunk_empty);
			return DESTROY_ME;
		}
		this->ike_sa->set_ike_cfg(this->ike_sa, ike_cfg);
		ike_cfg->destroy(ike_cfg);
		/* add a timeout if peer does not establish it completely */
		schedule_delete_job = TRUE;
	}
	this->ike_sa->set_statistic(this->ike_sa, STAT_INBOUND,
								time_monotonic(NULL));

	mid = msg->get_message_id(msg);
	if (msg->get_request(msg))
	{
		if (mid == this->responding.mid)
		{
			/* reject initial messages once established */
			if (msg->get_exchange_type(msg) == IKE_SA_INIT ||
				msg->get_exchange_type(msg) == IKE_AUTH)
			{
				if (this->ike_sa->get_state(this->ike_sa) != IKE_CREATED &&
					this->ike_sa->get_state(this->ike_sa) != IKE_CONNECTING)
				{
					DBG1(DBG_IKE, "ignoring %N in established IKE_SA state",
						 exchange_type_names, msg->get_exchange_type(msg));
					return FAILED;
				}
			}
			if (!this->ike_sa->supports_extension(this->ike_sa, EXT_MOBIKE))
			{	/* with MOBIKE, we do no implicit updates */
				this->ike_sa->update_hosts(this->ike_sa, me, other, mid == 1);
			}
			status = handle_fragment(this, &this->responding.defrag, msg);
			if (status != SUCCESS)
			{
				return status;
			}
			charon->bus->message(charon->bus, msg, TRUE, TRUE);
			if (msg->get_exchange_type(msg) == EXCHANGE_TYPE_UNDEFINED)
			{	/* ignore messages altered to EXCHANGE_TYPE_UNDEFINED */
				return SUCCESS;
			}
			if (process_request(this, msg) != SUCCESS)
			{
				flush(this);
				return DESTROY_ME;
			}
			this->responding.mid++;
		}
		else if ((mid == this->responding.mid - 1) &&
				 array_count(this->responding.packets))
		{
			status = handle_fragment(this, &this->responding.defrag, msg);
			if (status != SUCCESS)
			{
				return status;
			}
			DBG1(DBG_IKE, "received retransmit of request with ID %d, "
				 "retransmitting response", mid);
			charon->bus->alert(charon->bus, ALERT_RETRANSMIT_RECEIVE, msg);
			send_packets(this, this->responding.packets,
						 msg->get_destination(msg), msg->get_source(msg));
		}
		else
		{
			DBG1(DBG_IKE, "received message ID %d, expected %d. Ignored",
				 mid, this->responding.mid);
			if (msg->get_exchange_type(msg) == IKE_SA_INIT)
			{	/* clean up IKE_SA state if IKE_SA_INIT has invalid msg ID */
				return DESTROY_ME;
			}
		}
	}
	else
	{
		if (mid == this->initiating.mid)
		{
			if (this->ike_sa->get_state(this->ike_sa) == IKE_CREATED ||
				this->ike_sa->get_state(this->ike_sa) == IKE_CONNECTING ||
				msg->get_exchange_type(msg) != IKE_SA_INIT)
			{	/* only do updates based on verified messages (or initial ones) */
				if (!this->ike_sa->supports_extension(this->ike_sa, EXT_MOBIKE))
				{	/* with MOBIKE, we do no implicit updates.  we force an
					 * update of the local address on IKE_SA_INIT, but never
					 * for the remote address */
					this->ike_sa->update_hosts(this->ike_sa, me, NULL, mid == 0);
					this->ike_sa->update_hosts(this->ike_sa, NULL, other, FALSE);
				}
			}
			status = handle_fragment(this, &this->initiating.defrag, msg);
			if (status != SUCCESS)
			{
				return status;
			}
			charon->bus->message(charon->bus, msg, TRUE, TRUE);
			if (msg->get_exchange_type(msg) == EXCHANGE_TYPE_UNDEFINED)
			{	/* ignore messages altered to EXCHANGE_TYPE_UNDEFINED */
				return SUCCESS;
			}
			if (process_response(this, msg) != SUCCESS)
			{
				flush(this);
				return DESTROY_ME;
			}
		}
		else
		{
			DBG1(DBG_IKE, "received message ID %d, expected %d. Ignored",
				 mid, this->initiating.mid);
			return SUCCESS;
		}
	}

	if (schedule_delete_job)
	{
		ike_sa_id_t *ike_sa_id;
		job_t *job;

		ike_sa_id = this->ike_sa->get_id(this->ike_sa);
		job = (job_t*)delete_ike_sa_job_create(ike_sa_id, FALSE);
		lib->scheduler->schedule_job(lib->scheduler, job,
				lib->settings->get_int(lib->settings,
						"%s.half_open_timeout", HALF_OPEN_IKE_SA_TIMEOUT,
						lib->ns));
	}
	return SUCCESS;
}

METHOD(task_manager_t, queue_task, void,
	private_task_manager_t *this, task_t *task)
{
	if (task->get_type(task) == TASK_IKE_MOBIKE)
	{	/*  there is no need to queue more than one mobike task */
		enumerator_t *enumerator;
		task_t *current;

		enumerator = array_create_enumerator(this->queued_tasks);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (current->get_type(current) == TASK_IKE_MOBIKE)
			{
				enumerator->destroy(enumerator);
				task->destroy(task);
				return;
			}
		}
		enumerator->destroy(enumerator);
	}
	DBG2(DBG_IKE, "queueing %N task", task_type_names, task->get_type(task));
	array_insert(this->queued_tasks, ARRAY_TAIL, task);
}

/**
 * Check if a given task has been queued already
 */
static bool has_queued(private_task_manager_t *this, task_type_t type)
{
	enumerator_t *enumerator;
	bool found = FALSE;
	task_t *task;

	enumerator = array_create_enumerator(this->queued_tasks);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == type)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(task_manager_t, queue_ike, void,
	private_task_manager_t *this)
{
	if (!has_queued(this, TASK_IKE_VENDOR))
	{
		queue_task(this, (task_t*)ike_vendor_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_INIT))
	{
		queue_task(this, (task_t*)ike_init_create(this->ike_sa, TRUE, NULL));
	}
	if (!has_queued(this, TASK_IKE_NATD))
	{
		queue_task(this, (task_t*)ike_natd_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_CERT_PRE))
	{
		queue_task(this, (task_t*)ike_cert_pre_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_AUTH))
	{
		queue_task(this, (task_t*)ike_auth_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_CERT_POST))
	{
		queue_task(this, (task_t*)ike_cert_post_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_CONFIG))
	{
		queue_task(this, (task_t*)ike_config_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_AUTH_LIFETIME))
	{
		queue_task(this, (task_t*)ike_auth_lifetime_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_IKE_MOBIKE))
	{
		peer_cfg_t *peer_cfg;

		peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
		if (peer_cfg->use_mobike(peer_cfg))
		{
			queue_task(this, (task_t*)ike_mobike_create(this->ike_sa, TRUE));
		}
	}
#ifdef ME
	if (!has_queued(this, TASK_IKE_ME))
	{
		queue_task(this, (task_t*)ike_me_create(this->ike_sa, TRUE));
	}
#endif /* ME */
}

METHOD(task_manager_t, queue_ike_rekey, void,
	private_task_manager_t *this)
{
	queue_task(this, (task_t*)ike_rekey_create(this->ike_sa, TRUE));
}

METHOD(task_manager_t, queue_ike_reauth, void,
	private_task_manager_t *this)
{
	queue_task(this, (task_t*)ike_reauth_create(this->ike_sa));
}

METHOD(task_manager_t, queue_ike_delete, void,
	private_task_manager_t *this)
{
	queue_task(this, (task_t*)ike_delete_create(this->ike_sa, TRUE));
}

METHOD(task_manager_t, queue_mobike, void,
	private_task_manager_t *this, bool roam, bool address)
{
	ike_mobike_t *mobike;

	mobike = ike_mobike_create(this->ike_sa, TRUE);
	if (roam)
	{
		enumerator_t *enumerator;
		task_t *current;

		mobike->roam(mobike, address);

		/* enable path probing for a currently active MOBIKE task.  This might
		 * not be the case if an address appeared on a new interface while the
		 * current address is not working but has not yet disappeared. */
		enumerator = array_create_enumerator(this->active_tasks);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (current->get_type(current) == TASK_IKE_MOBIKE)
			{
				ike_mobike_t *active = (ike_mobike_t*)current;
				active->enable_probing(active);
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		mobike->addresses(mobike);
	}
	queue_task(this, &mobike->task);
}

METHOD(task_manager_t, queue_child, void,
	private_task_manager_t *this, child_cfg_t *cfg, u_int32_t reqid,
	traffic_selector_t *tsi, traffic_selector_t *tsr)
{
	child_create_t *task;

	task = child_create_create(this->ike_sa, cfg, FALSE, tsi, tsr);
	if (reqid)
	{
		task->use_reqid(task, reqid);
	}
	queue_task(this, &task->task);
}

METHOD(task_manager_t, queue_child_rekey, void,
	private_task_manager_t *this, protocol_id_t protocol, u_int32_t spi)
{
	queue_task(this, (task_t*)child_rekey_create(this->ike_sa, protocol, spi));
}

METHOD(task_manager_t, queue_child_delete, void,
	private_task_manager_t *this, protocol_id_t protocol, u_int32_t spi,
	bool expired)
{
	queue_task(this, (task_t*)child_delete_create(this->ike_sa,
												  protocol, spi, expired));
}

METHOD(task_manager_t, queue_dpd, void,
	private_task_manager_t *this)
{
	ike_mobike_t *mobike;

	if (this->ike_sa->supports_extension(this->ike_sa, EXT_MOBIKE) &&
		this->ike_sa->has_condition(this->ike_sa, COND_NAT_HERE))
	{
		/* use mobike enabled DPD to detect NAT mapping changes */
		mobike = ike_mobike_create(this->ike_sa, TRUE);
		mobike->dpd(mobike);
		queue_task(this, &mobike->task);
	}
	else
	{
		queue_task(this, (task_t*)ike_dpd_create(TRUE));
	}
}

METHOD(task_manager_t, adopt_tasks, void,
	private_task_manager_t *this, task_manager_t *other_public)
{
	private_task_manager_t *other = (private_task_manager_t*)other_public;
	task_t *task;

	/* move queued tasks from other to this */
	while (array_remove(other->queued_tasks, ARRAY_TAIL, &task))
	{
		DBG2(DBG_IKE, "migrating %N task", task_type_names, task->get_type(task));
		task->migrate(task, this->ike_sa);
		array_insert(this->queued_tasks, ARRAY_HEAD, task);
	}
}

/**
 * Migrates child-creating tasks from src to dst
 */
static void migrate_child_tasks(private_task_manager_t *this,
								array_t *src, array_t *dst)
{
	enumerator_t *enumerator;
	task_t *task;

	enumerator = array_create_enumerator(src);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_CHILD_CREATE)
		{
			array_remove_at(src, enumerator);
			task->migrate(task, this->ike_sa);
			array_insert(dst, ARRAY_TAIL, task);
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_manager_t, adopt_child_tasks, void,
	private_task_manager_t *this, task_manager_t *other_public)
{
	private_task_manager_t *other = (private_task_manager_t*)other_public;

	/* move active child tasks from other to this */
	migrate_child_tasks(this, other->active_tasks, this->queued_tasks);
	/* do the same for queued tasks */
	migrate_child_tasks(this, other->queued_tasks, this->queued_tasks);
}

METHOD(task_manager_t, busy, bool,
	private_task_manager_t *this)
{
	return array_count(this->active_tasks) > 0;
}

METHOD(task_manager_t, reset, void,
	private_task_manager_t *this, u_int32_t initiate, u_int32_t respond)
{
	enumerator_t *enumerator;
	task_t *task;

	/* reset message counters and retransmit packets */
	clear_packets(this->responding.packets);
	clear_packets(this->initiating.packets);
	DESTROY_IF(this->responding.defrag);
	DESTROY_IF(this->initiating.defrag);
	this->responding.defrag = NULL;
	this->initiating.defrag = NULL;
	if (initiate != UINT_MAX)
	{
		this->initiating.mid = initiate;
	}
	if (respond != UINT_MAX)
	{
		this->responding.mid = respond;
	}
	this->initiating.type = EXCHANGE_TYPE_UNDEFINED;

	/* reset queued tasks */
	enumerator = array_create_enumerator(this->queued_tasks);
	while (enumerator->enumerate(enumerator, &task))
	{
		task->migrate(task, this->ike_sa);
	}
	enumerator->destroy(enumerator);

	/* reset active tasks */
	while (array_remove(this->active_tasks, ARRAY_TAIL, &task))
	{
		task->migrate(task, this->ike_sa);
		array_insert(this->queued_tasks, ARRAY_HEAD, task);
	}

	this->reset = TRUE;
}

METHOD(task_manager_t, create_task_enumerator, enumerator_t*,
	private_task_manager_t *this, task_queue_t queue)
{
	switch (queue)
	{
		case TASK_QUEUE_ACTIVE:
			return array_create_enumerator(this->active_tasks);
		case TASK_QUEUE_PASSIVE:
			return array_create_enumerator(this->passive_tasks);
		case TASK_QUEUE_QUEUED:
			return array_create_enumerator(this->queued_tasks);
		default:
			return enumerator_create_empty();
	}
}

METHOD(task_manager_t, destroy, void,
	private_task_manager_t *this)
{
	flush(this);

	array_destroy(this->active_tasks);
	array_destroy(this->queued_tasks);
	array_destroy(this->passive_tasks);

	clear_packets(this->responding.packets);
	array_destroy(this->responding.packets);
	clear_packets(this->initiating.packets);
	array_destroy(this->initiating.packets);
	DESTROY_IF(this->responding.defrag);
	DESTROY_IF(this->initiating.defrag);
	free(this);
}

/*
 * see header file
 */
task_manager_v2_t *task_manager_v2_create(ike_sa_t *ike_sa)
{
	private_task_manager_t *this;

	INIT(this,
		.public = {
			.task_manager = {
				.process_message = _process_message,
				.queue_task = _queue_task,
				.queue_ike = _queue_ike,
				.queue_ike_rekey = _queue_ike_rekey,
				.queue_ike_reauth = _queue_ike_reauth,
				.queue_ike_delete = _queue_ike_delete,
				.queue_mobike = _queue_mobike,
				.queue_child = _queue_child,
				.queue_child_rekey = _queue_child_rekey,
				.queue_child_delete = _queue_child_delete,
				.queue_dpd = _queue_dpd,
				.initiate = _initiate,
				.retransmit = _retransmit,
				.incr_mid = _incr_mid,
				.reset = _reset,
				.adopt_tasks = _adopt_tasks,
				.adopt_child_tasks = _adopt_child_tasks,
				.busy = _busy,
				.create_task_enumerator = _create_task_enumerator,
				.flush = _flush,
				.flush_queue = _flush_queue,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.initiating.type = EXCHANGE_TYPE_UNDEFINED,
		.queued_tasks = array_create(0, 0),
		.active_tasks = array_create(0, 0),
		.passive_tasks = array_create(0, 0),
		.retransmit_tries = lib->settings->get_int(lib->settings,
					"%s.retransmit_tries", RETRANSMIT_TRIES, lib->ns),
		.retransmit_timeout = lib->settings->get_double(lib->settings,
					"%s.retransmit_timeout", RETRANSMIT_TIMEOUT, lib->ns),
		.retransmit_base = lib->settings->get_double(lib->settings,
					"%s.retransmit_base", RETRANSMIT_BASE, lib->ns),
	);

	return &this->public;
}

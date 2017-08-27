/*
 * Copyright (C) 2007-2014 Tobias Brunner
 * Copyright (C) 2007-2011 Martin Willi
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

#include "task_manager_v1.h"

#include <math.h>

#include <daemon.h>
#include <sa/ikev1/tasks/main_mode.h>
#include <sa/ikev1/tasks/aggressive_mode.h>
#include <sa/ikev1/tasks/quick_mode.h>
#include <sa/ikev1/tasks/quick_delete.h>
#include <sa/ikev1/tasks/xauth.h>
#include <sa/ikev1/tasks/mode_config.h>
#include <sa/ikev1/tasks/informational.h>
#include <sa/ikev1/tasks/isakmp_natd.h>
#include <sa/ikev1/tasks/isakmp_vendor.h>
#include <sa/ikev1/tasks/isakmp_cert_pre.h>
#include <sa/ikev1/tasks/isakmp_cert_post.h>
#include <sa/ikev1/tasks/isakmp_delete.h>
#include <sa/ikev1/tasks/isakmp_dpd.h>

#include <processing/jobs/retransmit_job.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/dpd_timeout_job.h>
#include <processing/jobs/process_message_job.h>

#include <collections/array.h>

/**
 * Number of old messages hashes we keep for retransmission.
 *
 * In Main Mode, we must ignore messages from a previous message pair if
 * we already continued to the next. Otherwise a late retransmission
 * could be considered as a reply to the newer request.
 */
#define MAX_OLD_HASHES 2

/**
 * First sequence number of responding packets.
 *
 * To distinguish retransmission jobs for initiating and responding packets,
 * we split up the sequence counter and use the upper half for responding.
 */
#define RESPONDING_SEQ INT_MAX

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
	task_manager_v1_t public;

	/**
	 * associated IKE_SA we are serving
	 */
	ike_sa_t *ike_sa;

	/**
	 * RNG to create message IDs
	 */
	rng_t *rng;

	/**
	 * Exchange we are currently handling as responder
	 */
	struct {
		/**
		 * Message ID of the last response
		 */
		u_int32_t mid;

		/**
		 * Hash of a previously received message
		 */
		u_int32_t hash;

		/**
		 * packet(s) for retransmission
		 */
		array_t *packets;

		/**
		 * Sequence number of the last sent message
		 */
		u_int32_t seqnr;

		/**
		 * how many times we have retransmitted so far
		 */
		u_int retransmitted;

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
		 * Hashes of old responses we can ignore
		 */
		u_int32_t old_hashes[MAX_OLD_HASHES];

		/**
		 * Position in old hash array
		 */
		int old_hash_pos;

		/**
		 * Sequence number of the last sent message
		 */
		u_int32_t seqnr;

		/**
		 * how many times we have retransmitted so far
		 */
		u_int retransmitted;

		/**
		 * packet(s) for retransmission
		 */
		array_t *packets;

		/**
		 * type of the initiated exchange
		 */
		exchange_type_t type;

	} initiating;

	/**
	 * Message we are currently defragmenting, if any (only one at a time)
	 */
	message_t *defrag;

	/**
	 * List of queued tasks not yet in action
	 */
	linked_list_t *queued_tasks;

	/**
	 * List of active tasks, initiated by ourselves
	 */
	linked_list_t *active_tasks;

	/**
	 * List of tasks initiated by peer
	 */
	linked_list_t *passive_tasks;

	/**
	 * Queued messages not yet ready to process
	 */
	message_t *queued;

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

	/**
	 * Sequence number for sending DPD requests
	 */
	u_int32_t dpd_send;

	/**
	 * Sequence number for received DPD requests
	 */
	u_int32_t dpd_recv;
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
	linked_list_t *list;
	task_t *task;

	if (this->queued)
	{
		this->queued->destroy(this->queued);
		this->queued = NULL;
	}
	switch (queue)
	{
		case TASK_QUEUE_ACTIVE:
			list = this->active_tasks;
			/* cancel pending retransmits */
			this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
			clear_packets(this->initiating.packets);
			break;
		case TASK_QUEUE_PASSIVE:
			list = this->passive_tasks;
			break;
		case TASK_QUEUE_QUEUED:
			list = this->queued_tasks;
			break;
		default:
			return;
	}
	while (list->remove_last(list, (void**)&task) == SUCCESS)
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

	enumerator = this->queued_tasks->create_enumerator(this->queued_tasks);
	while (enumerator->enumerate(enumerator, (void**)&task))
	{
		if (task->get_type(task) == type)
		{
			DBG2(DBG_IKE, "  activating %N task", task_type_names, type);
			this->queued_tasks->remove_at(this->queued_tasks, enumerator);
			this->active_tasks->insert_last(this->active_tasks, task);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Send packets in the given array (they get cloned)
 */
static void send_packets(private_task_manager_t *this, array_t *packets)
{
	enumerator_t *enumerator;
	packet_t *packet;

	enumerator = array_create_enumerator(packets);
	while (enumerator->enumerate(enumerator, &packet))
	{
		charon->sender->send(charon->sender, packet->clone(packet));
	}
	enumerator->destroy(enumerator);
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
	return TRUE;
}

/**
 * Retransmit a packet (or its fragments)
 */
static status_t retransmit_packet(private_task_manager_t *this, u_int32_t seqnr,
							u_int mid, u_int retransmitted, array_t *packets)
{
	packet_t *packet;
	u_int32_t t;

	array_get(packets, 0, &packet);
	if (retransmitted > this->retransmit_tries)
	{
		DBG1(DBG_IKE, "giving up after %u retransmits", retransmitted - 1);
		charon->bus->alert(charon->bus, ALERT_RETRANSMIT_SEND_TIMEOUT, packet);
		return DESTROY_ME;
	}
	t = (u_int32_t)(this->retransmit_timeout * 1000.0 *
					pow(this->retransmit_base, retransmitted));
	if (retransmitted)
	{
		DBG1(DBG_IKE, "sending retransmit %u of %s message ID %u, seq %u",
			 retransmitted, seqnr < RESPONDING_SEQ ? "request" : "response",
			 mid, seqnr < RESPONDING_SEQ ? seqnr : seqnr - RESPONDING_SEQ);
		charon->bus->alert(charon->bus, ALERT_RETRANSMIT_SEND, packet);
	}
	send_packets(this, packets);
	lib->scheduler->schedule_job_ms(lib->scheduler, (job_t*)
			retransmit_job_create(seqnr, this->ike_sa->get_id(this->ike_sa)), t);
	return NEED_MORE;
}

METHOD(task_manager_t, retransmit, status_t,
	private_task_manager_t *this, u_int32_t seqnr)
{
	status_t status = SUCCESS;

	if (seqnr == this->initiating.seqnr &&
		array_count(this->initiating.packets))
	{
		status = retransmit_packet(this, seqnr, this->initiating.mid,
					this->initiating.retransmitted, this->initiating.packets);
		if (status == NEED_MORE)
		{
			this->initiating.retransmitted++;
			status = SUCCESS;
		}
	}
	if (seqnr == this->responding.seqnr &&
		array_count(this->responding.packets))
	{
		status = retransmit_packet(this, seqnr, this->responding.mid,
					this->responding.retransmitted, this->responding.packets);
		if (status == NEED_MORE)
		{
			this->responding.retransmitted++;
			status = SUCCESS;
		}
	}
	return status;
}

/**
 * Check if we have to wait for a mode config before starting a quick mode
 */
static bool mode_config_expected(private_task_manager_t *this)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	char *pool;
	bool local;
	host_t *host;

	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer_cfg)
	{
		if (peer_cfg->use_pull_mode(peer_cfg))
		{
			enumerator = peer_cfg->create_pool_enumerator(peer_cfg);
			if (!enumerator->enumerate(enumerator, &pool))
			{	/* no pool configured */
				enumerator->destroy(enumerator);
				return FALSE;
			}
			enumerator->destroy(enumerator);

			local = FALSE;
		}
		else
		{
			enumerator = peer_cfg->create_virtual_ip_enumerator(peer_cfg);
			if (!enumerator->enumerate(enumerator, &host))
			{	/* not requesting a vip */
				enumerator->destroy(enumerator);
				return FALSE;
			}
			enumerator->destroy(enumerator);

			local = TRUE;
		}
		enumerator = this->ike_sa->create_virtual_ip_enumerator(this->ike_sa,
																local);
		if (!enumerator->enumerate(enumerator, &host))
		{	/* expecting a VIP exchange, but no VIP assigned yet */
			enumerator->destroy(enumerator);
			return TRUE;
		}
		enumerator->destroy(enumerator);
	}
	return FALSE;
}

METHOD(task_manager_t, initiate, status_t,
	private_task_manager_t *this)
{
	enumerator_t *enumerator;
	task_t *task;
	message_t *message;
	host_t *me, *other;
	exchange_type_t exchange = EXCHANGE_TYPE_UNDEFINED;
	bool new_mid = FALSE, expect_response = FALSE, cancelled = FALSE, keep = FALSE;

	if (this->initiating.type != EXCHANGE_TYPE_UNDEFINED &&
		this->initiating.type != INFORMATIONAL_V1)
	{
		DBG2(DBG_IKE, "delaying task initiation, %N exchange in progress",
				exchange_type_names, this->initiating.type);
		/* do not initiate if we already have a message in the air */
		return SUCCESS;
	}

	if (this->active_tasks->get_count(this->active_tasks) == 0)
	{
		DBG2(DBG_IKE, "activating new tasks");
		switch (this->ike_sa->get_state(this->ike_sa))
		{
			case IKE_CREATED:
				activate_task(this, TASK_ISAKMP_VENDOR);
				activate_task(this, TASK_ISAKMP_CERT_PRE);
				if (activate_task(this, TASK_MAIN_MODE))
				{
					exchange = ID_PROT;
				}
				else if (activate_task(this, TASK_AGGRESSIVE_MODE))
				{
					exchange = AGGRESSIVE;
				}
				activate_task(this, TASK_ISAKMP_CERT_POST);
				activate_task(this, TASK_ISAKMP_NATD);
				break;
			case IKE_CONNECTING:
				if (activate_task(this, TASK_ISAKMP_DELETE))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_XAUTH))
				{
					exchange = TRANSACTION;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_INFORMATIONAL))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				break;
			case IKE_ESTABLISHED:
				if (activate_task(this, TASK_MODE_CONFIG))
				{
					exchange = TRANSACTION;
					new_mid = TRUE;
					break;
				}
				if (!mode_config_expected(this) &&
					activate_task(this, TASK_QUICK_MODE))
				{
					exchange = QUICK_MODE;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_INFORMATIONAL))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_QUICK_DELETE))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_ISAKMP_DELETE))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				if (activate_task(this, TASK_ISAKMP_DPD))
				{
					exchange = INFORMATIONAL_V1;
					new_mid = TRUE;
					break;
				}
				break;
			default:
				break;
		}
	}
	else
	{
		DBG2(DBG_IKE, "reinitiating already active tasks");
		enumerator = this->active_tasks->create_enumerator(this->active_tasks);
		while (enumerator->enumerate(enumerator, (void**)&task))
		{
			DBG2(DBG_IKE, "  %N task", task_type_names, task->get_type(task));
			switch (task->get_type(task))
			{
				case TASK_MAIN_MODE:
					exchange = ID_PROT;
					break;
				case TASK_AGGRESSIVE_MODE:
					exchange = AGGRESSIVE;
					break;
				case TASK_QUICK_MODE:
					exchange = QUICK_MODE;
					break;
				case TASK_XAUTH:
					exchange = TRANSACTION;
					new_mid = TRUE;
					break;
				default:
					continue;
			}
			break;
		}
		enumerator->destroy(enumerator);
	}

	if (exchange == EXCHANGE_TYPE_UNDEFINED)
	{
		DBG2(DBG_IKE, "nothing to initiate");
		/* nothing to do yet... */
		return SUCCESS;
	}

	me = this->ike_sa->get_my_host(this->ike_sa);
	other = this->ike_sa->get_other_host(this->ike_sa);

	if (new_mid)
	{
		if (!this->rng->get_bytes(this->rng, sizeof(this->initiating.mid),
								 (void*)&this->initiating.mid))
		{
			DBG1(DBG_IKE, "failed to allocate message ID, destroying IKE_SA");
			flush(this);
			return DESTROY_ME;
		}
	}
	message = message_create(IKEV1_MAJOR_VERSION, IKEV1_MINOR_VERSION);
	message->set_message_id(message, this->initiating.mid);
	message->set_source(message, me->clone(me));
	message->set_destination(message, other->clone(other));
	message->set_exchange_type(message, exchange);
	this->initiating.type = exchange;
	this->initiating.retransmitted = 0;

	enumerator = this->active_tasks->create_enumerator(this->active_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->build(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				this->active_tasks->remove_at(this->active_tasks, enumerator);
				if (task->get_type(task) == TASK_AGGRESSIVE_MODE ||
					task->get_type(task) == TASK_QUICK_MODE)
				{	/* last message of three message exchange */
					keep = TRUE;
				}
				task->destroy(task);
				continue;
			case NEED_MORE:
				expect_response = TRUE;
				/* processed, but task needs another exchange */
				continue;
			case ALREADY_DONE:
				cancelled = TRUE;
				break;
			case FAILED:
			default:
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
		break;
	}
	enumerator->destroy(enumerator);

	if (this->active_tasks->get_count(this->active_tasks) == 0 &&
		(exchange == QUICK_MODE || exchange == AGGRESSIVE))
	{	/* tasks completed, no exchange active anymore */
		this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
	}
	if (cancelled)
	{
		message->destroy(message);
		return initiate(this);
	}

	clear_packets(this->initiating.packets);
	if (!generate_message(this, message, &this->initiating.packets))
	{
		/* message generation failed. There is nothing more to do than to
		 * close the SA */
		message->destroy(message);
		flush(this);
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}

	this->initiating.seqnr++;
	if (expect_response)
	{
		message->destroy(message);
		return retransmit(this, this->initiating.seqnr);
	}
	if (keep)
	{	/* keep the packet for retransmission, the responder might request it */
		send_packets(this, this->initiating.packets);
	}
	else
	{
		send_packets(this, this->initiating.packets);
		clear_packets(this->initiating.packets);
	}
	message->destroy(message);

	if (exchange == INFORMATIONAL_V1)
	{
		switch (this->ike_sa->get_state(this->ike_sa))
		{
			case IKE_CONNECTING:
				/* close after sending an INFORMATIONAL when unestablished */
				return FAILED;
			case IKE_DELETING:
				/* close after sending a DELETE */
				return DESTROY_ME;
			default:
				break;
		}
	}
	return initiate(this);
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
	bool delete = FALSE, cancelled = FALSE, expect_request = FALSE;

	me = request->get_destination(request);
	other = request->get_source(request);

	message = message_create(IKEV1_MAJOR_VERSION, IKEV1_MINOR_VERSION);
	message->set_exchange_type(message, request->get_exchange_type(request));
	/* send response along the path the request came in */
	message->set_source(message, me->clone(me));
	message->set_destination(message, other->clone(other));
	message->set_message_id(message, request->get_message_id(request));
	message->set_request(message, FALSE);

	this->responding.mid = request->get_message_id(request);
	this->responding.retransmitted = 0;
	this->responding.seqnr++;

	enumerator = this->passive_tasks->create_enumerator(this->passive_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->build(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				this->passive_tasks->remove_at(this->passive_tasks, enumerator);
				task->destroy(task);
				continue;
			case NEED_MORE:
				/* processed, but task needs another exchange */
				if (task->get_type(task) == TASK_QUICK_MODE ||
					task->get_type(task) == TASK_AGGRESSIVE_MODE)
				{	/* we rely on initiator retransmission, except for
					 * three-message exchanges */
					expect_request = TRUE;
				}
				continue;
			case ALREADY_DONE:
				cancelled = TRUE;
				break;
			case FAILED:
			default:
				charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				/* FALL */
			case DESTROY_ME:
				/* destroy IKE_SA, but SEND response first */
				delete = TRUE;
				break;
		}
		break;
	}
	enumerator->destroy(enumerator);

	clear_packets(this->responding.packets);
	if (cancelled)
	{
		message->destroy(message);
		return initiate(this);
	}
	if (!generate_message(this, message, &this->responding.packets))
	{
		message->destroy(message);
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}
	message->destroy(message);

	if (expect_request && !delete)
	{
		return retransmit(this, this->responding.seqnr);
	}
	send_packets(this, this->responding.packets);
	if (delete)
	{
		return DESTROY_ME;
	}
	return SUCCESS;
}

/**
 * Send a notify in a separate INFORMATIONAL exchange back to the sender.
 * The notify protocol_id is set to ISAKMP
 */
static void send_notify(private_task_manager_t *this, message_t *request,
						notify_type_t type)
{
	message_t *response;
	array_t *packets = NULL;
	host_t *me, *other;
	u_int32_t mid;

	if (request->get_exchange_type(request) == INFORMATIONAL_V1)
	{	/* don't respond to INFORMATIONAL requests to avoid a notify war */
		DBG1(DBG_IKE, "ignore malformed INFORMATIONAL request");
		return;
	}
	if (!this->rng->get_bytes(this->rng, sizeof(mid), (void*)&mid))
	{
		DBG1(DBG_IKE, "failed to allocate message ID");
		return;
	}
	response = message_create(IKEV1_MAJOR_VERSION, IKEV1_MINOR_VERSION);
	response->set_exchange_type(response, INFORMATIONAL_V1);
	response->set_request(response, TRUE);
	response->set_message_id(response, mid);
	response->add_payload(response, (payload_t*)
				notify_payload_create_from_protocol_and_type(PLV1_NOTIFY,
													PROTO_IKE, type));

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
	if (generate_message(this, response, &packets))
	{
		send_packets(this, packets);
	}
	clear_packets(packets);
	array_destroy(packets);
	response->destroy(response);
}

/**
 * Process a DPD request/response
 */
static bool process_dpd(private_task_manager_t *this, message_t *message)
{
	notify_payload_t *notify;
	notify_type_t type;
	u_int32_t seq;
	chunk_t data;

	type = DPD_R_U_THERE;
	notify = message->get_notify(message, type);
	if (!notify)
	{
		type = DPD_R_U_THERE_ACK;
		notify = message->get_notify(message, type);
	}
	if (!notify)
	{
		return FALSE;
	}
	data = notify->get_notification_data(notify);
	if (data.len != 4)
	{
		return FALSE;
	}
	seq = untoh32(data.ptr);

	if (type == DPD_R_U_THERE)
	{
		if (this->dpd_recv == 0 || seq == this->dpd_recv)
		{	/* check sequence validity */
			this->dpd_recv = seq + 1;
			this->ike_sa->set_statistic(this->ike_sa, STAT_INBOUND,
										time_monotonic(NULL));
		}
		/* but respond anyway */
		this->ike_sa->queue_task(this->ike_sa,
				&isakmp_dpd_create(this->ike_sa, DPD_R_U_THERE_ACK, seq)->task);
	}
	else /* DPD_R_U_THERE_ACK */
	{
		if (seq == this->dpd_send - 1)
		{
			this->ike_sa->set_statistic(this->ike_sa, STAT_INBOUND,
										time_monotonic(NULL));
		}
		else
		{
			DBG1(DBG_IKE, "received invalid DPD sequence number %u "
				 "(expected %u), ignored", seq, this->dpd_send - 1);
		}
	}
	return TRUE;
}

/**
 * handle an incoming request message
 */
static status_t process_request(private_task_manager_t *this,
								message_t *message)
{
	enumerator_t *enumerator;
	task_t *task = NULL;
	bool send_response = FALSE, dpd = FALSE;

	if (message->get_exchange_type(message) == INFORMATIONAL_V1 ||
		this->passive_tasks->get_count(this->passive_tasks) == 0)
	{	/* create tasks depending on request type, if not already some queued */
		switch (message->get_exchange_type(message))
		{
			case ID_PROT:
				task = (task_t *)isakmp_vendor_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t*)isakmp_cert_pre_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t *)main_mode_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t*)isakmp_cert_post_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t *)isakmp_natd_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				break;
			case AGGRESSIVE:
				task = (task_t *)isakmp_vendor_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t*)isakmp_cert_pre_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t *)aggressive_mode_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t*)isakmp_cert_post_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				task = (task_t *)isakmp_natd_create(this->ike_sa, FALSE);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				break;
			case QUICK_MODE:
				if (this->ike_sa->get_state(this->ike_sa) != IKE_ESTABLISHED)
				{
					DBG1(DBG_IKE, "received quick mode request for "
						 "unestablished IKE_SA, ignored");
					return FAILED;
				}
				task = (task_t *)quick_mode_create(this->ike_sa, NULL,
												   NULL, NULL);
				this->passive_tasks->insert_last(this->passive_tasks, task);
				break;
			case INFORMATIONAL_V1:
				if (process_dpd(this, message))
				{
					dpd = TRUE;
				}
				else
				{
					task = (task_t *)informational_create(this->ike_sa, NULL);
					this->passive_tasks->insert_first(this->passive_tasks, task);
				}
				break;
			case TRANSACTION:
				if (this->ike_sa->get_state(this->ike_sa) != IKE_CONNECTING)
				{
					task = (task_t *)mode_config_create(this->ike_sa,
														FALSE, TRUE);
				}
				else
				{
					task = (task_t *)xauth_create(this->ike_sa, FALSE);
				}
				this->passive_tasks->insert_last(this->passive_tasks, task);
				break;
			default:
				return FAILED;
		}
	}
	if (dpd)
	{
		return initiate(this);
	}
	this->ike_sa->set_statistic(this->ike_sa, STAT_INBOUND, time_monotonic(NULL));

	/* let the tasks process the message */
	enumerator = this->passive_tasks->create_enumerator(this->passive_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->process(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				this->passive_tasks->remove_at(this->passive_tasks, enumerator);
				task->destroy(task);
				continue;
			case NEED_MORE:
				/* processed, but task needs at least another call to build() */
				send_response = TRUE;
				continue;
			case ALREADY_DONE:
				send_response = FALSE;
				break;
			case FAILED:
			default:
				charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				/* FALL */
			case DESTROY_ME:
				/* critical failure, destroy IKE_SA */
				this->passive_tasks->remove_at(this->passive_tasks, enumerator);
				enumerator->destroy(enumerator);
				task->destroy(task);
				return DESTROY_ME;
		}
		break;
	}
	enumerator->destroy(enumerator);

	if (send_response)
	{
		if (build_response(this, message) != SUCCESS)
		{
			return DESTROY_ME;
		}
	}
	else
	{	/* We don't send a response, so don't retransmit one if we get
		 * the same message again. */
		clear_packets(this->responding.packets);
	}
	if (this->passive_tasks->get_count(this->passive_tasks) == 0 &&
		this->queued_tasks->get_count(this->queued_tasks) > 0)
	{
		/* passive tasks completed, check if an active task has been queued,
		 * such as XAUTH or modeconfig push */
		return initiate(this);
	}
	return SUCCESS;
}

/**
 * handle an incoming response message
 */
static status_t process_response(private_task_manager_t *this,
								 message_t *message)
{
	enumerator_t *enumerator;
	message_t *queued;
	status_t status;
	task_t *task;

	if (message->get_exchange_type(message) != this->initiating.type)
	{
		/* Windows server sends a fourth quick mode message having an initial
		 * contact notify. Ignore this message for compatibility. */
		if (this->initiating.type == EXCHANGE_TYPE_UNDEFINED &&
			message->get_exchange_type(message) == QUICK_MODE &&
			message->get_notify(message, INITIAL_CONTACT))
		{
			DBG1(DBG_IKE, "ignoring fourth Quick Mode message");
			return SUCCESS;
		}
		DBG1(DBG_IKE, "received %N response, but expected %N",
			 exchange_type_names, message->get_exchange_type(message),
			 exchange_type_names, this->initiating.type);
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
		return DESTROY_ME;
	}

	enumerator = this->active_tasks->create_enumerator(this->active_tasks);
	while (enumerator->enumerate(enumerator, (void*)&task))
	{
		switch (task->process(task, message))
		{
			case SUCCESS:
				/* task completed, remove it */
				this->active_tasks->remove_at(this->active_tasks, enumerator);
				task->destroy(task);
				continue;
			case NEED_MORE:
				/* processed, but task needs another exchange */
				continue;
			case ALREADY_DONE:
				break;
			case FAILED:
			default:
				charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
				/* FALL */
			case DESTROY_ME:
				/* critical failure, destroy IKE_SA */
				this->active_tasks->remove_at(this->active_tasks, enumerator);
				enumerator->destroy(enumerator);
				task->destroy(task);
				return DESTROY_ME;
		}
		break;
	}
	enumerator->destroy(enumerator);

	this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
	clear_packets(this->initiating.packets);

	if (this->queued && this->active_tasks->get_count(this->active_tasks) == 0)
	{
		queued = this->queued;
		this->queued = NULL;
		status = this->public.task_manager.process_message(
											&this->public.task_manager, queued);
		queued->destroy(queued);
		if (status == DESTROY_ME)
		{
			return status;
		}
	}

	return initiate(this);
}

static status_t handle_fragment(private_task_manager_t *this, message_t *msg)
{
	status_t status;

	if (!this->defrag)
	{
		this->defrag = message_create_defrag(msg);
		if (!this->defrag)
		{
			return FAILED;
		}
	}
	status = this->defrag->add_fragment(this->defrag, msg);
	if (status == SUCCESS)
	{
		lib->processor->queue_job(lib->processor,
							(job_t*)process_message_job_create(this->defrag));
		this->defrag = NULL;
		/* do not process the last fragment */
		status = NEED_MORE;
	}
	return status;
}

/**
 * Parse the given message and verify that it is valid.
 */
static status_t parse_message(private_task_manager_t *this, message_t *msg)
{
	status_t status;

	status = msg->parse_body(msg, this->ike_sa->get_keymat(this->ike_sa));

	if (status != SUCCESS)
	{
		switch (status)
		{
			case NOT_SUPPORTED:
				DBG1(DBG_IKE, "unsupported exchange type");
				send_notify(this, msg, INVALID_EXCHANGE_TYPE);
				break;
			case PARSE_ERROR:
				DBG1(DBG_IKE, "message parsing failed");
				send_notify(this, msg, PAYLOAD_MALFORMED);
				break;
			case VERIFY_ERROR:
				DBG1(DBG_IKE, "message verification failed");
				send_notify(this, msg, PAYLOAD_MALFORMED);
				break;
			case FAILED:
				DBG1(DBG_IKE, "integrity check failed");
				send_notify(this, msg, INVALID_HASH_INFORMATION);
				break;
			case INVALID_STATE:
				DBG1(DBG_IKE, "found encrypted message, but no keys available");
				send_notify(this, msg, PAYLOAD_MALFORMED);
			default:
				break;
		}
		DBG1(DBG_IKE, "%N %s with message ID %u processing failed",
			 exchange_type_names, msg->get_exchange_type(msg),
			 msg->get_request(msg) ? "request" : "response",
			 msg->get_message_id(msg));

		charon->bus->alert(charon->bus, ALERT_PARSE_ERROR_BODY, msg, status);

		if (this->ike_sa->get_state(this->ike_sa) == IKE_CREATED)
		{	/* invalid initiation attempt, close SA */
			return DESTROY_ME;
		}
	}

	if (msg->get_first_payload_type(msg) == PLV1_FRAGMENT)
	{
		return handle_fragment(this, msg);
	}
	return status;
}

METHOD(task_manager_t, process_message, status_t,
	private_task_manager_t *this, message_t *msg)
{
	u_int32_t hash, mid, i;
	host_t *me, *other;
	status_t status;

	/* TODO-IKEv1: update hosts more selectively */
	me = msg->get_destination(msg);
	other = msg->get_source(msg);
	mid = msg->get_message_id(msg);
	hash = chunk_hash(msg->get_packet_data(msg));
	for (i = 0; i < MAX_OLD_HASHES; i++)
	{
		if (this->initiating.old_hashes[i] == hash)
		{
			if (array_count(this->initiating.packets) &&
				i == (this->initiating.old_hash_pos % MAX_OLD_HASHES) &&
				(msg->get_exchange_type(msg) == QUICK_MODE ||
				 msg->get_exchange_type(msg) == AGGRESSIVE))
			{
				DBG1(DBG_IKE, "received retransmit of response with ID %u, "
					 "resending last request", mid);
				send_packets(this, this->initiating.packets);
				return SUCCESS;
			}
			DBG1(DBG_IKE, "received retransmit of response with ID %u, "
				 "but next request already sent", mid);
			return SUCCESS;
		}
	}

	if ((mid && mid == this->initiating.mid) ||
		(this->initiating.mid == 0 &&
		 msg->get_exchange_type(msg) == this->initiating.type &&
		 this->active_tasks->get_count(this->active_tasks)))
	{
		msg->set_request(msg, FALSE);
		charon->bus->message(charon->bus, msg, TRUE, FALSE);
		status = parse_message(this, msg);
		if (status == NEED_MORE)
		{
			return SUCCESS;
		}
		if (status != SUCCESS)
		{
			return status;
		}
		this->ike_sa->set_statistic(this->ike_sa, STAT_INBOUND,
									time_monotonic(NULL));
		this->ike_sa->update_hosts(this->ike_sa, me, other, TRUE);
		charon->bus->message(charon->bus, msg, TRUE, TRUE);
		if (process_response(this, msg) != SUCCESS)
		{
			flush(this);
			return DESTROY_ME;
		}
		this->initiating.old_hashes[(++this->initiating.old_hash_pos) %
									MAX_OLD_HASHES] = hash;
	}
	else
	{
		if (hash == this->responding.hash)
		{
			if (array_count(this->responding.packets))
			{
				DBG1(DBG_IKE, "received retransmit of request with ID %u, "
					 "retransmitting response", mid);
				send_packets(this, this->responding.packets);
			}
			else if (array_count(this->initiating.packets) &&
					 this->initiating.type == INFORMATIONAL_V1)
			{
				DBG1(DBG_IKE, "received retransmit of DPD request, "
					 "retransmitting response");
				send_packets(this, this->initiating.packets);
			}
			else
			{
				DBG1(DBG_IKE, "received retransmit of request with ID %u, "
					 "but no response to retransmit", mid);
			}
			charon->bus->alert(charon->bus, ALERT_RETRANSMIT_RECEIVE, msg);
			return SUCCESS;
		}

		/* reject Main/Aggressive Modes once established */
		if (msg->get_exchange_type(msg) == ID_PROT ||
			msg->get_exchange_type(msg) == AGGRESSIVE)
		{
			if (this->ike_sa->get_state(this->ike_sa) != IKE_CREATED &&
				this->ike_sa->get_state(this->ike_sa) != IKE_CONNECTING &&
				msg->get_first_payload_type(msg) != PLV1_FRAGMENT)
			{
				DBG1(DBG_IKE, "ignoring %N in established IKE_SA state",
					 exchange_type_names, msg->get_exchange_type(msg));
				return FAILED;
			}
		}

		if (msg->get_exchange_type(msg) == TRANSACTION &&
			this->active_tasks->get_count(this->active_tasks))
		{	/* main mode not yet complete, queue XAuth/Mode config tasks */
			if (this->queued)
			{
				DBG1(DBG_IKE, "ignoring additional %N request, queue full",
					 exchange_type_names, TRANSACTION);
				return SUCCESS;
			}
			this->queued = message_create_from_packet(msg->get_packet(msg));
			if (this->queued->parse_header(this->queued) != SUCCESS)
			{
				this->queued->destroy(this->queued);
				this->queued = NULL;
				return FAILED;
			}
			DBG1(DBG_IKE, "queueing %N request as tasks still active",
				 exchange_type_names, TRANSACTION);
			return SUCCESS;
		}

		msg->set_request(msg, TRUE);
		charon->bus->message(charon->bus, msg, TRUE, FALSE);
		status = parse_message(this, msg);
		if (status == NEED_MORE)
		{
			return SUCCESS;
		}
		if (status != SUCCESS)
		{
			return status;
		}
		/* if this IKE_SA is virgin, we check for a config */
		if (this->ike_sa->get_ike_cfg(this->ike_sa) == NULL)
		{
			ike_sa_id_t *ike_sa_id;
			ike_cfg_t *ike_cfg;
			job_t *job;

			ike_cfg = charon->backends->get_ike_cfg(charon->backends,
													me, other, IKEV1);
			if (ike_cfg == NULL)
			{
				/* no config found for these hosts, destroy */
				DBG1(DBG_IKE, "no IKE config found for %H...%H, sending %N",
					 me, other, notify_type_names, NO_PROPOSAL_CHOSEN);
				send_notify(this, msg, NO_PROPOSAL_CHOSEN);
				return DESTROY_ME;
			}
			this->ike_sa->set_ike_cfg(this->ike_sa, ike_cfg);
			ike_cfg->destroy(ike_cfg);
			/* add a timeout if peer does not establish it completely */
			ike_sa_id = this->ike_sa->get_id(this->ike_sa);
			job = (job_t*)delete_ike_sa_job_create(ike_sa_id, FALSE);
			lib->scheduler->schedule_job(lib->scheduler, job,
					lib->settings->get_int(lib->settings,
							"%s.half_open_timeout", HALF_OPEN_IKE_SA_TIMEOUT,
							lib->ns));
		}
		this->ike_sa->update_hosts(this->ike_sa, me, other, TRUE);
		charon->bus->message(charon->bus, msg, TRUE, TRUE);
		if (process_request(this, msg) != SUCCESS)
		{
			flush(this);
			return DESTROY_ME;
		}
		this->responding.hash = hash;
	}
	return SUCCESS;
}

/**
 * Check if a given task has been queued already
 */
static bool has_queued(private_task_manager_t *this, task_type_t type)
{
	enumerator_t *enumerator;
	bool found = FALSE;
	task_t *task;

	enumerator = this->queued_tasks->create_enumerator(this->queued_tasks);
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

METHOD(task_manager_t, queue_task, void,
	private_task_manager_t *this, task_t *task)
{
	task_type_t type = task->get_type(task);

	switch (type)
	{
		case TASK_MODE_CONFIG:
		case TASK_XAUTH:
			if (has_queued(this, type))
			{
				task->destroy(task);
				return;
			}
			break;
		default:
			break;
	}
	DBG2(DBG_IKE, "queueing %N task", task_type_names, task->get_type(task));
	this->queued_tasks->insert_last(this->queued_tasks, task);
}

METHOD(task_manager_t, queue_ike, void,
	private_task_manager_t *this)
{
	peer_cfg_t *peer_cfg;

	if (!has_queued(this, TASK_ISAKMP_VENDOR))
	{
		queue_task(this, (task_t*)isakmp_vendor_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_ISAKMP_CERT_PRE))
	{
		queue_task(this, (task_t*)isakmp_cert_pre_create(this->ike_sa, TRUE));
	}
	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer_cfg->use_aggressive(peer_cfg))
	{
		if (!has_queued(this, TASK_AGGRESSIVE_MODE))
		{
			queue_task(this, (task_t*)aggressive_mode_create(this->ike_sa, TRUE));
		}
	}
	else
	{
		if (!has_queued(this, TASK_MAIN_MODE))
		{
			queue_task(this, (task_t*)main_mode_create(this->ike_sa, TRUE));
		}
	}
	if (!has_queued(this, TASK_ISAKMP_CERT_POST))
	{
		queue_task(this, (task_t*)isakmp_cert_post_create(this->ike_sa, TRUE));
	}
	if (!has_queued(this, TASK_ISAKMP_NATD))
	{
		queue_task(this, (task_t*)isakmp_natd_create(this->ike_sa, TRUE));
	}
}

METHOD(task_manager_t, queue_ike_reauth, void,
	private_task_manager_t *this)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	ike_sa_t *new;
	host_t *host;

	new = charon->ike_sa_manager->checkout_new(charon->ike_sa_manager,
								this->ike_sa->get_version(this->ike_sa), TRUE);
	if (!new)
	{	/* shouldn't happen */
		return;
	}

	new->set_peer_cfg(new, this->ike_sa->get_peer_cfg(this->ike_sa));
	host = this->ike_sa->get_other_host(this->ike_sa);
	new->set_other_host(new, host->clone(host));
	host = this->ike_sa->get_my_host(this->ike_sa);
	new->set_my_host(new, host->clone(host));
	enumerator = this->ike_sa->create_virtual_ip_enumerator(this->ike_sa, TRUE);
	while (enumerator->enumerate(enumerator, &host))
	{
		new->add_virtual_ip(new, TRUE, host);
	}
	enumerator->destroy(enumerator);

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		this->ike_sa->remove_child_sa(this->ike_sa, enumerator);
		new->add_child_sa(new, child_sa);
	}
	enumerator->destroy(enumerator);

	if (!new->get_child_count(new))
	{	/* check if a Quick Mode task is queued (UNITY_LOAD_BALANCE case) */
		task_t *task;

		enumerator = this->queued_tasks->create_enumerator(this->queued_tasks);
		while (enumerator->enumerate(enumerator, &task))
		{
			if (task->get_type(task) == TASK_QUICK_MODE)
			{
				this->queued_tasks->remove_at(this->queued_tasks, enumerator);
				task->migrate(task, new);
				new->queue_task(new, task);
			}
		}
		enumerator->destroy(enumerator);
	}

	if (new->initiate(new, NULL, 0, NULL, NULL) != DESTROY_ME)
	{
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, new);
		this->ike_sa->set_state(this->ike_sa, IKE_REKEYING);
	}
	else
	{
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, new);
		DBG1(DBG_IKE, "reauthenticating IKE_SA failed");
	}
	charon->bus->set_sa(charon->bus, this->ike_sa);
}

METHOD(task_manager_t, queue_ike_rekey, void,
	private_task_manager_t *this)
{
	queue_ike_reauth(this);
}

METHOD(task_manager_t, queue_ike_delete, void,
	private_task_manager_t *this)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa;

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		queue_task(this, (task_t*)
			quick_delete_create(this->ike_sa, child_sa->get_protocol(child_sa),
								child_sa->get_spi(child_sa, TRUE), FALSE, FALSE));
	}
	enumerator->destroy(enumerator);

	queue_task(this, (task_t*)isakmp_delete_create(this->ike_sa, TRUE));
}

METHOD(task_manager_t, queue_mobike, void,
	private_task_manager_t *this, bool roam, bool address)
{
	/* Not supported in IKEv1 */
}

METHOD(task_manager_t, queue_child, void,
	private_task_manager_t *this, child_cfg_t *cfg, u_int32_t reqid,
	traffic_selector_t *tsi, traffic_selector_t *tsr)
{
	quick_mode_t *task;

	task = quick_mode_create(this->ike_sa, cfg, tsi, tsr);
	task->use_reqid(task, reqid);

	queue_task(this, &task->task);
}

/**
 * Check if two CHILD_SAs have the same traffic selector
 */
static bool have_equal_ts(child_sa_t *child1, child_sa_t *child2, bool local)
{
	enumerator_t *e1, *e2;
	traffic_selector_t *ts1, *ts2;
	bool equal = FALSE;

	e1 = child1->create_ts_enumerator(child1, local);
	e2 = child2->create_ts_enumerator(child2, local);
	if (e1->enumerate(e1, &ts1) && e2->enumerate(e2, &ts2))
	{
		equal = ts1->equals(ts1, ts2);
	}
	e2->destroy(e2);
	e1->destroy(e1);

	return equal;
}

/**
 * Check if a CHILD_SA is redundant and we should delete instead of rekey
 */
static bool is_redundant(private_task_manager_t *this, child_sa_t *child_sa)
{
	enumerator_t *enumerator;
	child_sa_t *current;
	bool redundant = FALSE;

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current->get_state(current) == CHILD_INSTALLED &&
			streq(current->get_name(current), child_sa->get_name(child_sa)) &&
			have_equal_ts(current, child_sa, TRUE) &&
			have_equal_ts(current, child_sa, FALSE) &&
			current->get_lifetime(current, FALSE) >
				child_sa->get_lifetime(child_sa, FALSE))
		{
			DBG1(DBG_IKE, "deleting redundant CHILD_SA %s{%d}",
				 child_sa->get_name(child_sa), child_sa->get_reqid(child_sa));
			redundant = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return redundant;
}

/**
 * Get the first traffic selector of a CHILD_SA, local or remote
 */
static traffic_selector_t* get_first_ts(child_sa_t *child_sa, bool local)
{
	traffic_selector_t *ts = NULL;
	enumerator_t *enumerator;

	enumerator = child_sa->create_ts_enumerator(child_sa, local);
	enumerator->enumerate(enumerator, &ts);
	enumerator->destroy(enumerator);

	return ts;
}

METHOD(task_manager_t, queue_child_rekey, void,
	private_task_manager_t *this, protocol_id_t protocol, u_int32_t spi)
{
	child_sa_t *child_sa;
	child_cfg_t *cfg;
	quick_mode_t *task;

	child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol, spi, TRUE);
	if (!child_sa)
	{
		child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol, spi, FALSE);
	}
	if (child_sa && child_sa->get_state(child_sa) == CHILD_INSTALLED)
	{
		if (is_redundant(this, child_sa))
		{
			queue_task(this, (task_t*)quick_delete_create(this->ike_sa,
												protocol, spi, FALSE, FALSE));
		}
		else
		{
			child_sa->set_state(child_sa, CHILD_REKEYING);
			cfg = child_sa->get_config(child_sa);
			task = quick_mode_create(this->ike_sa, cfg->get_ref(cfg),
				get_first_ts(child_sa, TRUE), get_first_ts(child_sa, FALSE));
			task->use_reqid(task, child_sa->get_reqid(child_sa));
			task->rekey(task, child_sa->get_spi(child_sa, TRUE));

			queue_task(this, &task->task);
		}
	}
}

METHOD(task_manager_t, queue_child_delete, void,
	private_task_manager_t *this, protocol_id_t protocol, u_int32_t spi,
	bool expired)
{
	queue_task(this, (task_t*)quick_delete_create(this->ike_sa, protocol,
												  spi, FALSE, expired));
}

METHOD(task_manager_t, queue_dpd, void,
	private_task_manager_t *this)
{
	peer_cfg_t *peer_cfg;
	u_int32_t t, retransmit;

	queue_task(this, (task_t*)isakmp_dpd_create(this->ike_sa, DPD_R_U_THERE,
												this->dpd_send++));
	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);

	/* compute timeout in milliseconds */
	t = 1000 * peer_cfg->get_dpd_timeout(peer_cfg);
	if (t == 0)
	{
		/* use the same timeout as a retransmitting IKE message would have */
		for (retransmit = 0; retransmit <= this->retransmit_tries; retransmit++)
		{
			t += (u_int32_t)(this->retransmit_timeout * 1000.0 *
							pow(this->retransmit_base, retransmit));
		}
	}

	/* schedule DPD timeout job */
	lib->scheduler->schedule_job_ms(lib->scheduler,
		(job_t*)dpd_timeout_job_create(this->ike_sa->get_id(this->ike_sa)), t);
}

METHOD(task_manager_t, adopt_tasks, void,
	private_task_manager_t *this, task_manager_t *other_public)
{
	private_task_manager_t *other = (private_task_manager_t*)other_public;
	task_t *task;

	/* move queued tasks from other to this */
	while (other->queued_tasks->remove_last(other->queued_tasks,
												(void**)&task) == SUCCESS)
	{
		DBG2(DBG_IKE, "migrating %N task", task_type_names, task->get_type(task));
		task->migrate(task, this->ike_sa);
		this->queued_tasks->insert_first(this->queued_tasks, task);
	}
}

/**
 * Migrates child-creating tasks from src to dst
 */
static void migrate_child_tasks(private_task_manager_t *this,
								linked_list_t *src, linked_list_t *dst)
{
	enumerator_t *enumerator;
	task_t *task;

	enumerator = src->create_enumerator(src);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_QUICK_MODE)
		{
			src->remove_at(src, enumerator);
			task->migrate(task, this->ike_sa);
			dst->insert_last(dst, task);
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
	return (this->active_tasks->get_count(this->active_tasks) > 0);
}

METHOD(task_manager_t, incr_mid, void,
	private_task_manager_t *this, bool initiate)
{
}

METHOD(task_manager_t, reset, void,
	private_task_manager_t *this, u_int32_t initiate, u_int32_t respond)
{
	enumerator_t *enumerator;
	task_t *task;

	/* reset message counters and retransmit packets */
	clear_packets(this->responding.packets);
	clear_packets(this->initiating.packets);
	this->responding.seqnr = RESPONDING_SEQ;
	this->responding.retransmitted = 0;
	this->initiating.mid = 0;
	this->initiating.seqnr = 0;
	this->initiating.retransmitted = 0;
	this->initiating.type = EXCHANGE_TYPE_UNDEFINED;
	DESTROY_IF(this->defrag);
	this->defrag = NULL;
	if (initiate != UINT_MAX)
	{
		this->dpd_send = initiate;
	}
	if (respond != UINT_MAX)
	{
		this->dpd_recv = respond;
	}

	/* reset queued tasks */
	enumerator = this->queued_tasks->create_enumerator(this->queued_tasks);
	while (enumerator->enumerate(enumerator, &task))
	{
		task->migrate(task, this->ike_sa);
	}
	enumerator->destroy(enumerator);

	/* reset active tasks */
	while (this->active_tasks->remove_last(this->active_tasks,
										   (void**)&task) == SUCCESS)
	{
		task->migrate(task, this->ike_sa);
		this->queued_tasks->insert_first(this->queued_tasks, task);
	}
}

METHOD(task_manager_t, create_task_enumerator, enumerator_t*,
	private_task_manager_t *this, task_queue_t queue)
{
	switch (queue)
	{
		case TASK_QUEUE_ACTIVE:
			return this->active_tasks->create_enumerator(this->active_tasks);
		case TASK_QUEUE_PASSIVE:
			return this->passive_tasks->create_enumerator(this->passive_tasks);
		case TASK_QUEUE_QUEUED:
			return this->queued_tasks->create_enumerator(this->queued_tasks);
		default:
			return enumerator_create_empty();
	}
}

METHOD(task_manager_t, destroy, void,
	private_task_manager_t *this)
{
	flush(this);

	this->active_tasks->destroy(this->active_tasks);
	this->queued_tasks->destroy(this->queued_tasks);
	this->passive_tasks->destroy(this->passive_tasks);
	DESTROY_IF(this->defrag);

	DESTROY_IF(this->queued);
	clear_packets(this->responding.packets);
	array_destroy(this->responding.packets);
	clear_packets(this->initiating.packets);
	array_destroy(this->initiating.packets);
	DESTROY_IF(this->rng);
	free(this);
}

/*
 * see header file
 */
task_manager_v1_t *task_manager_v1_create(ike_sa_t *ike_sa)
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
		.initiating = {
			.type = EXCHANGE_TYPE_UNDEFINED,
		},
		.responding = {
			.seqnr = RESPONDING_SEQ,
		},
		.ike_sa = ike_sa,
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
		.queued_tasks = linked_list_create(),
		.active_tasks = linked_list_create(),
		.passive_tasks = linked_list_create(),
		.retransmit_tries = lib->settings->get_int(lib->settings,
						"%s.retransmit_tries", RETRANSMIT_TRIES, lib->ns),
		.retransmit_timeout = lib->settings->get_double(lib->settings,
						"%s.retransmit_timeout", RETRANSMIT_TIMEOUT, lib->ns),
		.retransmit_base = lib->settings->get_double(lib->settings,
						"%s.retransmit_base", RETRANSMIT_BASE, lib->ns),
	);

	if (!this->rng)
	{
		DBG1(DBG_IKE, "no RNG found, unable to create IKE_SA");
		destroy(this);
		return NULL;
	}
	if (!this->rng->get_bytes(this->rng, sizeof(this->dpd_send),
							  (void*)&this->dpd_send))
	{
		DBG1(DBG_IKE, "failed to allocate message ID, unable to create IKE_SA");
		destroy(this);
		return NULL;
	}
	this->dpd_send &= 0x7FFFFFFF;

	return &this->public;
}

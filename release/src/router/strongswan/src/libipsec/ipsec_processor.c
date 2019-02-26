/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "ipsec.h"
#include "ipsec_processor.h"

#include <utils/debug.h>
#include <library.h>
#include <threading/rwlock.h>
#include <collections/blocking_queue.h>
#include <processing/jobs/callback_job.h>

typedef struct private_ipsec_processor_t private_ipsec_processor_t;

/**
 * Private additions to ipsec_processor_t.
 */
struct private_ipsec_processor_t {

	/**
	 * Public members
	 */
	ipsec_processor_t public;

	/**
	 * Queue for inbound packets (esp_packet_t*)
	 */
	blocking_queue_t *inbound_queue;

	/**
	 * Queue for outbound packets (ip_packet_t*)
	 */
	blocking_queue_t *outbound_queue;

	/**
	 * Registered inbound callback
	 */
	struct {
		ipsec_inbound_cb_t cb;
		void *data;
	} inbound;

	/**
	 * Registered outbound callback
	 */
	struct {
		ipsec_outbound_cb_t cb;
		void *data;
	} outbound;

	/**
	 * Lock used to synchronize access to the callbacks
	 */
	rwlock_t *lock;
};

/**
 * Deliver an inbound IP packet to the registered listener
 */
static void deliver_inbound(private_ipsec_processor_t *this,
							esp_packet_t *packet)
{
	this->lock->read_lock(this->lock);
	if (this->inbound.cb)
	{
		this->inbound.cb(this->inbound.data, packet->extract_payload(packet));
	}
	else
	{
		DBG2(DBG_ESP, "no inbound callback registered, dropping packet");
	}
	packet->destroy(packet);
	this->lock->unlock(this->lock);
}

/**
 * Processes inbound packets
 */
static job_requeue_t process_inbound(private_ipsec_processor_t *this)
{
	esp_packet_t *packet;
	ip_packet_t *ip_packet;
	ipsec_sa_t *sa;
	uint8_t next_header;
	uint32_t spi, reqid;

	packet = (esp_packet_t*)this->inbound_queue->dequeue(this->inbound_queue);

	if (!packet->parse_header(packet, &spi))
	{
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}

	sa = ipsec->sas->checkout_by_spi(ipsec->sas, spi,
									 packet->get_destination(packet));
	if (!sa)
	{
		DBG2(DBG_ESP, "inbound ESP packet does not belong to an installed SA");
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}

	if (!sa->is_inbound(sa))
	{
		DBG1(DBG_ESP, "error: IPsec SA is not inbound");
		packet->destroy(packet);
		ipsec->sas->checkin(ipsec->sas, sa);
		return JOB_REQUEUE_DIRECT;
	}

	if (packet->decrypt(packet, sa->get_esp_context(sa)) != SUCCESS)
	{
		ipsec->sas->checkin(ipsec->sas, sa);
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}
	ip_packet = packet->get_payload(packet);
	sa->update_usestats(sa, ip_packet->get_encoding(ip_packet).len);
	reqid = sa->get_reqid(sa);
	ipsec->sas->checkin(ipsec->sas, sa);

	next_header = packet->get_next_header(packet);
	switch (next_header)
	{
		case IPPROTO_IPIP:
		case IPPROTO_IPV6:
		{
			ipsec_policy_t *policy;

			policy = ipsec->policies->find_by_packet(ipsec->policies,
													 ip_packet, TRUE, reqid);
			if (policy)
			{
				deliver_inbound(this, packet);
				policy->destroy(policy);
				break;
			}
			DBG1(DBG_ESP, "discarding inbound IP packet %#H == %#H [%hhu] due "
				 "to policy", ip_packet->get_source(ip_packet),
				 ip_packet->get_destination(ip_packet),
				 ip_packet->get_next_header(ip_packet));
			/* no matching policy found, fall-through */
		}
		case IPPROTO_NONE:
			/* discard dummy packets */
			/* fall-through */
		default:
			packet->destroy(packet);
			break;
	}
	return JOB_REQUEUE_DIRECT;
}

/**
 * Send an ESP packet using the registered outbound callback
 */
static void send_outbound(private_ipsec_processor_t *this,
						  esp_packet_t *packet)
{
	this->lock->read_lock(this->lock);
	if (this->outbound.cb)
	{
		this->outbound.cb(this->outbound.data, packet);
	}
	else
	{
		DBG2(DBG_ESP, "no outbound callback registered, dropping packet");
		packet->destroy(packet);
	}
	this->lock->unlock(this->lock);
}

/**
 * Processes outbound packets
 */
static job_requeue_t process_outbound(private_ipsec_processor_t *this)
{
	ipsec_policy_t *policy;
	esp_packet_t *esp_packet;
	ip_packet_t *packet;
	ipsec_sa_t *sa;
	host_t *src, *dst;

	packet = (ip_packet_t*)this->outbound_queue->dequeue(this->outbound_queue);

	policy = ipsec->policies->find_by_packet(ipsec->policies, packet, FALSE, 0);
	if (!policy)
	{
		DBG2(DBG_ESP, "no matching outbound IPsec policy for %#H == %#H [%hhu]",
			 packet->get_source(packet), packet->get_destination(packet),
			 packet->get_next_header(packet));
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}

	sa = ipsec->sas->checkout_by_reqid(ipsec->sas, policy->get_reqid(policy),
									   FALSE);
	if (!sa)
	{	/* TODO-IPSEC: send an acquire to uppper layer */
		DBG1(DBG_ESP, "could not find an outbound IPsec SA for reqid {%u}, "
			 "dropping packet", policy->get_reqid(policy));
		packet->destroy(packet);
		policy->destroy(policy);
		return JOB_REQUEUE_DIRECT;
	}
	src = sa->get_source(sa);
	dst = sa->get_destination(sa);
	esp_packet = esp_packet_create_from_payload(src->clone(src),
												dst->clone(dst), packet);
	if (esp_packet->encrypt(esp_packet, sa->get_esp_context(sa),
							sa->get_spi(sa)) != SUCCESS)
	{
		ipsec->sas->checkin(ipsec->sas, sa);
		esp_packet->destroy(esp_packet);
		policy->destroy(policy);
		return JOB_REQUEUE_DIRECT;
	}
	sa->update_usestats(sa, packet->get_encoding(packet).len);
	ipsec->sas->checkin(ipsec->sas, sa);
	policy->destroy(policy);
	send_outbound(this, esp_packet);
	return JOB_REQUEUE_DIRECT;
}

METHOD(ipsec_processor_t, queue_inbound, void,
	private_ipsec_processor_t *this, esp_packet_t *packet)
{
	this->inbound_queue->enqueue(this->inbound_queue, packet);
}

METHOD(ipsec_processor_t, queue_outbound, void,
	private_ipsec_processor_t *this, ip_packet_t *packet)
{
	this->outbound_queue->enqueue(this->outbound_queue, packet);
}

METHOD(ipsec_processor_t, register_inbound, void,
	private_ipsec_processor_t *this, ipsec_inbound_cb_t cb, void *data)
{
	this->lock->write_lock(this->lock);
	this->inbound.cb = cb;
	this->inbound.data = data;
	this->lock->unlock(this->lock);
}

METHOD(ipsec_processor_t, unregister_inbound, void,
	private_ipsec_processor_t *this, ipsec_inbound_cb_t cb)
{
	this->lock->write_lock(this->lock);
	if (this->inbound.cb == cb)
	{
		this->inbound.cb = NULL;
	}
	this->lock->unlock(this->lock);
}

METHOD(ipsec_processor_t, register_outbound, void,
	private_ipsec_processor_t *this, ipsec_outbound_cb_t cb, void *data)
{
	this->lock->write_lock(this->lock);
	this->outbound.cb = cb;
	this->outbound.data = data;
	this->lock->unlock(this->lock);
}

METHOD(ipsec_processor_t, unregister_outbound, void,
	private_ipsec_processor_t *this, ipsec_outbound_cb_t cb)
{
	this->lock->write_lock(this->lock);
	if (this->outbound.cb == cb)
	{
		this->outbound.cb = NULL;
	}
	this->lock->unlock(this->lock);
}

METHOD(ipsec_processor_t, destroy, void,
	private_ipsec_processor_t *this)
{
	this->inbound_queue->destroy_offset(this->inbound_queue,
										offsetof(esp_packet_t, destroy));
	this->outbound_queue->destroy_offset(this->outbound_queue,
										 offsetof(ip_packet_t, destroy));
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Described in header.
 */
ipsec_processor_t *ipsec_processor_create()
{
	private_ipsec_processor_t *this;

	INIT(this,
		.public = {
			.queue_inbound = _queue_inbound,
			.queue_outbound = _queue_outbound,
			.register_inbound = _register_inbound,
			.unregister_inbound = _unregister_inbound,
			.register_outbound = _register_outbound,
			.unregister_outbound = _unregister_outbound,
			.destroy = _destroy,
		},
		.inbound_queue = blocking_queue_create(),
		.outbound_queue = blocking_queue_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create((callback_job_cb_t)process_inbound, this,
									NULL, (callback_job_cancel_t)return_false));
	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create((callback_job_cb_t)process_outbound, this,
									NULL, (callback_job_cancel_t)return_false));
	return &this->public;
}

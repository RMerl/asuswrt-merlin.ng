/*
 * Copyright (C) 2010-2020 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#include "ike_mobike.h"

#include <string.h>

#include <daemon.h>
#include <sa/ikev2/tasks/ike_natd.h>
#include <encoding/payloads/notify_payload.h>

#define COOKIE2_SIZE 16
#define MAX_ADDITIONAL_ADDRS 8

typedef struct private_ike_mobike_t private_ike_mobike_t;

/**
 * Private members of a ike_mobike_t task.
 */
struct private_ike_mobike_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_mobike_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * cookie2 value to verify new addresses
	 */
	chunk_t cookie2;

	/**
	 * NAT discovery reusing the TASK_IKE_NATD task
	 */
	ike_natd_t *natd;

	/**
	 * use task to update addresses
	 */
	bool update;

	/**
	 * do routability check
	 */
	bool check;

	/**
	 * include address list update
	 */
	bool address;

	/**
	 * additional addresses got updated
	 */
	bool addresses_updated;
};

/**
 * Check if a newer MOBIKE update task is queued
 */
static bool is_newer_update_queued(private_ike_mobike_t *this)
{
	enumerator_t *enumerator;
	private_ike_mobike_t *mobike;
	task_t *task;
	bool found = FALSE;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_QUEUED);
	while (enumerator->enumerate(enumerator, &task))
	{
		if (task->get_type(task) == TASK_IKE_MOBIKE)
		{
			mobike = (private_ike_mobike_t*)task;
			/* a queued check or update might invalidate the results of the
			 * current task */
			found = mobike->check || mobike->update;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * read notifys from message and evaluate them
 */
static void process_payloads(private_ike_mobike_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool first = TRUE;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		int family = AF_INET;
		notify_payload_t *notify;
		chunk_t data;
		host_t *host;

		if (payload->get_type(payload) != PLV2_NOTIFY)
		{
			continue;
		}
		notify = (notify_payload_t*)payload;
		switch (notify->get_notify_type(notify))
		{
			case MOBIKE_SUPPORTED:
			{
				peer_cfg_t *peer_cfg;

				peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
				if (!this->initiator &&
					peer_cfg && !peer_cfg->use_mobike(peer_cfg))
				{
					DBG1(DBG_IKE, "peer supports MOBIKE, but disabled in config");
				}
				else
				{
					DBG1(DBG_IKE, "peer supports MOBIKE");
					this->ike_sa->enable_extension(this->ike_sa, EXT_MOBIKE);
				}
				break;
			}
			case COOKIE2:
			{
				chunk_free(&this->cookie2);
				this->cookie2 = chunk_clone(notify->get_notification_data(notify));
				break;
			}
			case ADDITIONAL_IP6_ADDRESS:
			{
				family = AF_INET6;
				/* fall through */
			}
			case ADDITIONAL_IP4_ADDRESS:
			{
				if (first)
				{	/* an ADDITIONAL_*_ADDRESS means replace, so flush once */
					this->ike_sa->clear_peer_addresses(this->ike_sa);
					first = FALSE;
					/* add the peer's current address to the list */
					host = message->get_source(message);
					this->ike_sa->add_peer_address(this->ike_sa,
												   host->clone(host));
				}
				data = notify->get_notification_data(notify);
				host = host_create_from_chunk(family, data, 0);
				DBG2(DBG_IKE, "got additional MOBIKE peer address: %H", host);
				this->ike_sa->add_peer_address(this->ike_sa, host);
				this->addresses_updated = TRUE;
				break;
			}
			case UPDATE_SA_ADDRESSES:
			{
				this->update = TRUE;
				break;
			}
			case NO_ADDITIONAL_ADDRESSES:
			{
				this->ike_sa->clear_peer_addresses(this->ike_sa);
				/* add the peer's current address to the list */
				host = message->get_source(message);
				this->ike_sa->add_peer_address(this->ike_sa, host->clone(host));
				this->addresses_updated = TRUE;
				break;
			}
			case NAT_DETECTION_SOURCE_IP:
			case NAT_DETECTION_DESTINATION_IP:
			{
				/* NAT check in this MOBIKE exchange, create subtask for it */
				if (!this->natd)
				{
					this->natd = ike_natd_create(this->ike_sa, this->initiator);
				}
				break;
			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add ADDITIONAL_*_ADDRESS notifys depending on our address list
 */
static void build_address_list(private_ike_mobike_t *this, message_t *message)
{
	enumerator_t *enumerator;
	host_t *host, *me;
	notify_type_t type;
	int added = 0;

	me = this->ike_sa->get_my_host(this->ike_sa);
	enumerator = charon->kernel->create_address_enumerator(charon->kernel,
														   ADDR_TYPE_REGULAR);
	while (enumerator->enumerate(enumerator, (void**)&host))
	{
		if (me->ip_equals(me, host))
		{	/* "ADDITIONAL" means do not include IKE_SAs host */
			continue;
		}
		switch (host->get_family(host))
		{
			case AF_INET:
				type = ADDITIONAL_IP4_ADDRESS;
				break;
			case AF_INET6:
				type = ADDITIONAL_IP6_ADDRESS;
				break;
			default:
				continue;
		}
		message->add_notify(message, FALSE, type, host->get_address(host));
		if (++added >= MAX_ADDITIONAL_ADDRS)
		{	/* limit number of notifys, some implementations do not like too
			 * many of them (f.e. strongSwan ;-) */
			break;
		}
	}
	if (!added)
	{
		message->add_notify(message, FALSE, NO_ADDITIONAL_ADDRESSES, chunk_empty);
	}
	enumerator->destroy(enumerator);
}

/**
 * build a cookie and add it to the message
 */
static bool build_cookie(private_ike_mobike_t *this, message_t *message)
{
	rng_t *rng;

	chunk_free(&this->cookie2);
	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng || !rng->allocate_bytes(rng, COOKIE2_SIZE, &this->cookie2))
	{
		DESTROY_IF(rng);
		return FALSE;
	}
	message->add_notify(message, FALSE, COOKIE2, this->cookie2);
	rng->destroy(rng);
	return TRUE;
}

/**
 * Apply the port of the old host, if its ip equals the new, use port otherwise.
 */
static void apply_port(host_t *host, host_t *old, uint16_t port, bool local)
{
	if (host->ip_equals(host, old))
	{
		port = old->get_port(old);
	}
	else if (local && port == charon->socket->get_port(charon->socket, FALSE))
	{
		port = charon->socket->get_port(charon->socket, TRUE);
	}
	else if (!local && port == IKEV2_UDP_PORT)
	{
		port = IKEV2_NATT_PORT;
	}
	host->set_port(host, port);
}

METHOD(ike_mobike_t, transmit, bool,
	   private_ike_mobike_t *this, packet_t *packet)
{
	host_t *me, *other, *me_old, *other_old;
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	packet_t *copy;
	int family = AF_UNSPEC;
	bool found = FALSE;

	me_old = this->ike_sa->get_my_host(this->ike_sa);
	other_old = this->ike_sa->get_other_host(this->ike_sa);
	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	if (!this->check)
	{
		me = charon->kernel->get_source_addr(charon->kernel, other_old, me_old);
		if (me)
		{
			if (me->ip_equals(me, me_old))
			{
				copy = packet->clone(packet);
				/* hosts might have been updated by a peer's MOBIKE exchange */
				copy->set_source(copy, me_old->clone(me_old));
				copy->set_destination(copy, other_old->clone(other_old));
				charon->sender->send(charon->sender, copy);
				me->destroy(me);
				return TRUE;
			}
			me->destroy(me);
		}
		this->check = TRUE;
	}

	switch (charon->socket->supported_families(charon->socket))
	{
		case SOCKET_FAMILY_IPV4:
			family = AF_INET;
			break;
		case SOCKET_FAMILY_IPV6:
			family = AF_INET6;
			break;
		case SOCKET_FAMILY_BOTH:
		case SOCKET_FAMILY_NONE:
			break;
	}

	enumerator = this->ike_sa->create_peer_address_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, (void**)&other))
	{
		if (family != AF_UNSPEC && other->get_family(other) != family)
		{
			continue;
		}
		me = charon->kernel->get_source_addr(charon->kernel, other, NULL);
		if (me)
		{
			/* reuse port for an active address, 4500 otherwise */
			apply_port(me, me_old, ike_cfg->get_my_port(ike_cfg), TRUE);
			other = other->clone(other);
			apply_port(other, other_old, ike_cfg->get_other_port(ike_cfg), FALSE);
			DBG1(DBG_IKE, "checking path %#H - %#H", me, other);
			copy = packet->clone(packet);
			copy->set_source(copy, me);
			copy->set_destination(copy, other);
			charon->sender->send(charon->sender, copy);
			found = TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

METHOD(task_t, build_i, status_t,
	   private_ike_mobike_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH)
	{
		message->add_notify(message, FALSE, MOBIKE_SUPPORTED, chunk_empty);
		build_address_list(this, message);
		/* only in first IKE_AUTH */
		this->public.task.build = (void*)return_need_more;
	}
	else if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		host_t *old, *new;

		/* this task might have been queued before we knew if MOBIKE will be
		 * supported */
		if (!this->ike_sa->supports_extension(this->ike_sa, EXT_MOBIKE))
		{
			message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
			return SUCCESS;
		}

		/* we check if the existing address is still valid */
		old = message->get_source(message);
		new = charon->kernel->get_source_addr(charon->kernel,
										message->get_destination(message), old);
		if (new)
		{
			if (!new->ip_equals(new, old))
			{
				new->set_port(new, old->get_port(old));
				message->set_source(message, new);
			}
			else
			{
				new->destroy(new);
			}
		}
		if (this->update)
		{
			message->add_notify(message, FALSE, UPDATE_SA_ADDRESSES,
								chunk_empty);
			if (!build_cookie(this, message))
			{
				return FAILED;
			}
		}
		if (this->address && !this->check)
		{
			build_address_list(this, message);
		}
		if (this->natd)
		{
			this->natd->task.build(&this->natd->task, message);
		}
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	   private_ike_mobike_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH)
	{
		process_payloads(this, message);
		/* only first IKE_AUTH */
		this->public.task.process = (void*)return_need_more;
	}
	else if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		host_t *me_new = NULL, *other, *other_old, *other_new = NULL;

		process_payloads(this, message);

		if (this->natd)
		{
			this->natd->task.process(&this->natd->task, message);
		}

		if (this->update)
		{
			me_new = message->get_destination(message);
			other_new = message->get_source(message);
		}
		else if (this->addresses_updated &&
				 this->ike_sa->has_condition(this->ike_sa, COND_ORIGINAL_INITIATOR))
		{
			other = message->get_source(message);
			other_old = this->ike_sa->get_other_host(this->ike_sa);
			if (!other->equals(other, other_old))
			{
				other_new = other;
				/* our address might have changed too if the responder used
				 * a different address from our list to reach us */
				me_new = message->get_destination(message);
			}
		}

		if (me_new || other_new)
		{
			this->ike_sa->update_hosts(this->ike_sa, me_new,
									   other_new, UPDATE_HOSTS_FORCE_ALL);
		}
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	   private_ike_mobike_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
	{	/* in last IKE_AUTH only */
		if (this->ike_sa->supports_extension(this->ike_sa, EXT_MOBIKE))
		{
			message->add_notify(message, FALSE, MOBIKE_SUPPORTED, chunk_empty);
			build_address_list(this, message);
		}
		return SUCCESS;
	}
	else if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		if (this->natd)
		{
			this->natd->task.build(&this->natd->task, message);
		}
		if (this->cookie2.ptr)
		{
			message->add_notify(message, FALSE, COOKIE2, this->cookie2);
			chunk_free(&this->cookie2);
		}
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	   private_ike_mobike_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
	{	/* in last IKE_AUTH only */
		process_payloads(this, message);
		return SUCCESS;
	}
	else if (message->get_exchange_type(message) == INFORMATIONAL)
	{
		bool force = FALSE;

		if (is_newer_update_queued(this))
		{
			return SUCCESS;
		}
		if (this->cookie2.ptr)
		{	/* check cookie if we included one */
			chunk_t cookie2;

			cookie2 = this->cookie2;
			this->cookie2 = chunk_empty;
			process_payloads(this, message);
			if (!chunk_equals_const(cookie2, this->cookie2))
			{
				chunk_free(&cookie2);
				DBG1(DBG_IKE, "COOKIE2 mismatch, closing IKE_SA");
				return FAILED;
			}
			chunk_free(&cookie2);
		}
		else
		{
			process_payloads(this, message);
		}
		if (this->natd)
		{
			this->natd->task.process(&this->natd->task, message);

			if (this->update)
			{	/* update children again, as NAT state may have changed */
				this->ike_sa->update_hosts(this->ike_sa, NULL, NULL,
										   UPDATE_HOSTS_FORCE_CHILDREN);
			}
			else if (this->natd->has_mapping_changed(this->natd))
			{	/* force a check/update if mappings have changed during a DPD */
				force = TRUE;
				this->check = TRUE;
				DBG1(DBG_IKE, "detected changes in NAT mappings, "
					 "initiating MOBIKE update");
			}
		}
		if (this->check)
		{
			host_t *me, *me_new = NULL, *other, *other_new = NULL;

			me = message->get_destination(message);
			other = message->get_source(message);

			if (!me->equals(me, this->ike_sa->get_my_host(this->ike_sa)))
			{
				me_new = me;
			}
			if (!other->equals(other, this->ike_sa->get_other_host(this->ike_sa)))
			{
				other_new = other;
			}
			if (me_new || other_new || force)
			{
				this->ike_sa->update_hosts(this->ike_sa, me_new, other_new,
										   UPDATE_HOSTS_FORCE_ALL);
				/* use the same task to ... */
				if (!this->ike_sa->has_condition(this->ike_sa,
												 COND_ORIGINAL_INITIATOR))
				{	/*... send an updated list of addresses as responder */
					this->address = TRUE;
				}
				else
				{	/* ... send the update as original initiator */
					this->update = TRUE;
					if (this->natd)
					{
						this->natd->task.destroy(&this->natd->task);
					}
					this->natd = ike_natd_create(this->ike_sa, this->initiator);
				}
				this->check = FALSE;
				return NEED_MORE;
			}
		}
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(ike_mobike_t, addresses, void,
	   private_ike_mobike_t *this)
{
	this->address = TRUE;
}

METHOD(ike_mobike_t, roam, void,
	   private_ike_mobike_t *this, bool address)
{
	this->check = TRUE;
	this->address |= address;
}

METHOD(ike_mobike_t, dpd, void,
	   private_ike_mobike_t *this)
{
	if (!this->natd && this->ike_sa->has_condition(this->ike_sa, COND_NAT_HERE))
	{
		this->natd = ike_natd_create(this->ike_sa, this->initiator);
	}
}

METHOD(ike_mobike_t, is_probing, bool,
	   private_ike_mobike_t *this)
{
	return this->check;
}

METHOD(ike_mobike_t, enable_probing, void,
	private_ike_mobike_t *this)
{
	this->check = TRUE;
}

METHOD(task_t, get_type, task_type_t,
	   private_ike_mobike_t *this)
{
	return TASK_IKE_MOBIKE;
}

METHOD(task_t, migrate, void,
	   private_ike_mobike_t *this, ike_sa_t *ike_sa)
{
	chunk_free(&this->cookie2);
	this->ike_sa = ike_sa;
	if (this->natd)
	{
		this->natd->task.migrate(&this->natd->task, ike_sa);
	}
	this->public.task.build = _build_i;
}

METHOD(task_t, destroy, void,
	   private_ike_mobike_t *this)
{
	chunk_free(&this->cookie2);
	if (this->natd)
	{
		this->natd->task.destroy(&this->natd->task);
	}
	free(this);
}

/*
 * Described in header.
 */
ike_mobike_t *ike_mobike_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_mobike_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.addresses = _addresses,
			.roam = _roam,
			.dpd = _dpd,
			.transmit = _transmit,
			.is_probing = _is_probing,
			.enable_probing = _enable_probing,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
	);

	if (initiator)
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

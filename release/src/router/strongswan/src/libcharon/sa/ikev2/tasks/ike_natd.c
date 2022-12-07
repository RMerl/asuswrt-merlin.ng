/*
 * Copyright (C) 2006-2007 Martin Willi
 * Copyright (C) 2006 Tobias Brunner, Daniel Roethlisberger
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

#include "ike_natd.h"

#include <string.h>

#include <daemon.h>
#include <config/peer_cfg.h>
#include <crypto/hashers/hasher.h>
#include <encoding/payloads/notify_payload.h>
#include <sa/ikev2/tasks/ike_mobike.h>

typedef struct private_ike_natd_t private_ike_natd_t;

/**
 * Private members of a ike_natd_t task.
 */
struct private_ike_natd_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_natd_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Hasher used to build NAT detection hashes
	 */
	hasher_t *hasher;

	/**
	 * Did we process any NAT detection notifies for a source address?
	 */
	bool src_seen;

	/**
	 * Did we process any NAT detection notifies for a destination address?
	 */
	bool dst_seen;

	/**
	 * Have we found a matching source address NAT hash?
	 */
	bool src_matched;

	/**
	 * Have we found a matching destination address NAT hash?
	 */
	bool dst_matched;

	/**
	 * Whether NAT mappings for our NATed address has changed
	 */
	bool mapping_changed;

	/**
	 * Whether we faked a NAT situation because we didn't know our own IP
	 */
	bool fake_no_source_ip;
};

/**
 * Check if UDP encapsulation has to be forced either by config or required
 * by the kernel interface
 */
static bool force_encap(ike_cfg_t *ike_cfg)
{
	if (!ike_cfg->force_encap(ike_cfg))
	{
		return charon->kernel->get_features(charon->kernel) &
					KERNEL_REQUIRE_UDP_ENCAPSULATION;
	}
	return TRUE;
}

/**
 * Build NAT detection hash for a host
 */
static chunk_t generate_natd_hash(private_ike_natd_t *this,
								  ike_sa_id_t *ike_sa_id, host_t *host)
{
	chunk_t natd_chunk, spi_i_chunk, spi_r_chunk, addr_chunk, port_chunk;
	chunk_t natd_hash;
	uint64_t spi_i, spi_r;
	uint16_t port;

	/* prepare all required chunks */
	spi_i = ike_sa_id->get_initiator_spi(ike_sa_id);
	spi_r = ike_sa_id->get_responder_spi(ike_sa_id);
	spi_i_chunk.ptr = (void*)&spi_i;
	spi_i_chunk.len = sizeof(spi_i);
	spi_r_chunk.ptr = (void*)&spi_r;
	spi_r_chunk.len = sizeof(spi_r);
	port = htons(host->get_port(host));
	port_chunk.ptr = (void*)&port;
	port_chunk.len = sizeof(port);
	addr_chunk = host->get_address(host);

	/*  natd_hash = SHA1( spi_i | spi_r | address | port ) */
	natd_chunk = chunk_cat("cccc", spi_i_chunk, spi_r_chunk, addr_chunk, port_chunk);
	if (!this->hasher->allocate_hash(this->hasher, natd_chunk, &natd_hash))
	{
		natd_hash = chunk_empty;
	}
	DBG3(DBG_IKE, "natd_chunk %B", &natd_chunk);
	DBG3(DBG_IKE, "natd_hash %B", &natd_hash);

	chunk_free(&natd_chunk);
	return natd_hash;
}

/**
 * Build a NAT detection notify payload.
 */
static notify_payload_t *build_natd_payload(private_ike_natd_t *this,
											notify_type_t type, host_t *host)
{
	chunk_t hash;
	notify_payload_t *notify;
	ike_sa_id_t *ike_sa_id;
	ike_cfg_t *config;

	ike_sa_id = this->ike_sa->get_id(this->ike_sa);
	config = this->ike_sa->get_ike_cfg(this->ike_sa);
	if (type == NAT_DETECTION_SOURCE_IP && (force_encap(config) || host->is_anyaddr(host)))
	{
		uint32_t addr;

		/* chunk_hash() is randomly keyed so this produces a random IPv4 address
		 * that changes with every restart but otherwise stays the same */
		addr = chunk_hash(chunk_from_chars(0x00, 0x00, 0x00, 0x00));
		host = host_create_from_chunk(AF_INET, chunk_from_thing(addr), 0);
		hash = generate_natd_hash(this, ike_sa_id, host);
		host->destroy(host);
	}
	else
	{
		hash = generate_natd_hash(this, ike_sa_id, host);
	}
	if (!hash.len)
	{
		return NULL;
	}
	notify = notify_payload_create(PLV2_NOTIFY);
	notify->set_notify_type(notify, type);
	notify->set_notification_data(notify, hash);
	chunk_free(&hash);

	return notify;
}

/**
 * Process NAT detection notifies
 */
static void process_payloads(private_ike_natd_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	notify_payload_t *notify;
	chunk_t hash, src_hash, dst_hash;
	ike_sa_id_t *ike_sa_id;
	host_t *me, *other;
	ike_cfg_t *config;

	/* Precompute NAT-D hashes for incoming NAT notify comparison */
	ike_sa_id = message->get_ike_sa_id(message);
	me = message->get_destination(message);
	other = message->get_source(message);
	dst_hash = generate_natd_hash(this, ike_sa_id, me);
	src_hash = generate_natd_hash(this, ike_sa_id, other);

	DBG3(DBG_IKE, "precalculated src_hash %B", &src_hash);
	DBG3(DBG_IKE, "precalculated dst_hash %B", &dst_hash);

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) != PLV2_NOTIFY)
		{
			continue;
		}
		notify = (notify_payload_t*)payload;
		switch (notify->get_notify_type(notify))
		{
			case NAT_DETECTION_DESTINATION_IP:
			{
				this->dst_seen = TRUE;
				hash = notify->get_notification_data(notify);
				if (!this->dst_matched)
				{
					DBG3(DBG_IKE, "received dst_hash %B", &hash);
					if (chunk_equals(hash, dst_hash))
					{
						this->dst_matched = TRUE;
					}
				}
				/* RFC4555 says we should also compare against IKE_SA_INIT
				 * NATD payloads, but this does not work: We are running
				 * there at port 500, but use 4500 afterwards... */
				if (message->get_exchange_type(message) == INFORMATIONAL &&
					this->initiator && !this->dst_matched)
				{
					this->mapping_changed = this->ike_sa->has_mapping_changed(
															this->ike_sa, hash);
				}
				break;
			}
			case NAT_DETECTION_SOURCE_IP:
			{
				this->src_seen = TRUE;
				if (!this->src_matched)
				{
					hash = notify->get_notification_data(notify);
					DBG3(DBG_IKE, "received src_hash %B", &hash);
					if (chunk_equals(hash, src_hash))
					{
						this->src_matched = TRUE;
					}
				}
				break;
			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	chunk_free(&src_hash);
	chunk_free(&dst_hash);

	if (this->src_seen && this->dst_seen)
	{
		this->ike_sa->enable_extension(this->ike_sa, EXT_NATT);

		if (this->fake_no_source_ip)
		{
			if (this->dst_matched)
			{	/* after we faked a NAT, we now see that we are not behind one.
				 * we still have to register this as NAT to enable UDP-encap but
				 * we do so as actual local NAT, so DPDs will contain NAT-D
				 * payloads and we can remove this condition later via MOBIKE */
				this->dst_matched = FALSE;
			}
			else
			{	/* we are behind a NAT anyway, so nothing special to be done */
				this->fake_no_source_ip = FALSE;
			}
		}
		else if (this->initiator && this->dst_matched &&
				 this->ike_sa->has_condition(this->ike_sa, COND_NAT_HERE))
		{	/* we are not behind a NAT anymore, which might be because we
			 * didn't know our source IP initially. this flag triggers a MOBIKE
			 * update during a DPD, but it's not checked during MOBIKE updates
			 * when moving from behind a NAT to a public address */
			this->mapping_changed = TRUE;
		}

		this->ike_sa->set_condition(this->ike_sa, COND_NAT_HERE,
									!this->dst_matched);
		this->ike_sa->set_condition(this->ike_sa, COND_NAT_THERE,
									!this->src_matched);
		config = this->ike_sa->get_ike_cfg(this->ike_sa);
		if (this->dst_matched && this->src_matched &&
			force_encap(config))
		{
			this->ike_sa->set_condition(this->ike_sa, COND_NAT_FAKE, TRUE);
		}
	}
}

METHOD(task_t, process_i, status_t,
	private_ike_natd_t *this, message_t *message)
{
	process_payloads(this, message);

	if (message->get_exchange_type(message) == IKE_SA_INIT)
	{
		peer_cfg_t *peer_cfg;
		ike_mobike_t *mobike;

		peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
		if (this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY) ||
			(peer_cfg->use_mobike(peer_cfg) &&
			 this->ike_sa->supports_extension(this->ike_sa, EXT_NATT)))
		{	/* if the peer supports NAT-T, we switch to port 4500 even if no
			 * NAT is detected. can't be done later (when we would know
			 * whether the peer supports MOBIKE) because there is no exchange
			 * to actually do the switch (other than a forced DPD) */
			this->ike_sa->float_ports(this->ike_sa);
		}

		if (this->fake_no_source_ip)
		{	/* the NAT-D payloads revealed that we are not behind a NAT, so
			 * queue a MOBIKE-enabled DPD to check if UDP-encap can actually be
			 * disabled once we know if MOBIKE is supported */
			mobike = ike_mobike_create(this->ike_sa, TRUE);
			mobike->dpd(mobike);
			this->ike_sa->queue_task(this->ike_sa, &mobike->task);
		}
	}
	return SUCCESS;
}

METHOD(task_t, build_i, status_t,
	private_ike_natd_t *this, message_t *message)
{
	notify_payload_t *notify;
	ike_cfg_t *ike_cfg;
	host_t *host;

	if (this->hasher == NULL)
	{
		DBG1(DBG_IKE, "unable to build NATD payloads, SHA1 not supported");
		return NEED_MORE;
	}

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	host = message->get_source(message);
	if (!host->is_anyaddr(host) || force_encap(ike_cfg))
	{
		notify = build_natd_payload(this, NAT_DETECTION_SOURCE_IP, host);
		if (notify)
		{
			message->add_payload(message, (payload_t*)notify);
		}
	}
	else
	{	/* if we were not able to determine the source address (e.g. because
		 * DHCP is not done yet), we default to forcing encap for IPv4 and
		 * disabling NAT-D for IPv6 by not sending any notifies */
		if (host->get_family(host) == AF_INET)
		{
			DBG1(DBG_IKE, "unable to determine source address, faking NAT "
				 "situation");
			this->fake_no_source_ip = TRUE;
			notify = build_natd_payload(this, NAT_DETECTION_SOURCE_IP, host);
			if (notify)
			{
				message->add_payload(message, (payload_t*)notify);
			}
		}
		else
		{
			DBG1(DBG_IKE, "unable to determine source address, disabling NAT-D");
		}
	}

	if (message->get_notify(message, NAT_DETECTION_SOURCE_IP))
	{
		host = message->get_destination(message);
		notify = build_natd_payload(this, NAT_DETECTION_DESTINATION_IP, host);
		if (notify)
		{
			message->add_payload(message, (payload_t*)notify);
		}
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_natd_t *this, message_t *message)
{
	notify_payload_t *notify;
	host_t *me, *other;

	/* only add notifies on successful responses. */
	if (message->get_exchange_type(message) == IKE_SA_INIT &&
		message->get_payload(message, PLV2_SECURITY_ASSOCIATION) == NULL)
	{
		return SUCCESS;
	}

	if (this->src_seen && this->dst_seen)
	{
		if (this->hasher == NULL)
		{
			DBG1(DBG_IKE, "unable to build NATD payloads, SHA1 not supported");
			return SUCCESS;
		}

		/* initiator seems to support NAT detection, add response */
		me = message->get_source(message);
		notify = build_natd_payload(this, NAT_DETECTION_SOURCE_IP, me);
		if (notify)
		{
			message->add_payload(message, (payload_t*)notify);
		}
		other = message->get_destination(message);
		notify = build_natd_payload(this, NAT_DETECTION_DESTINATION_IP, other);
		if (notify)
		{
			message->add_payload(message, (payload_t*)notify);
		}
	}
	return SUCCESS;
}

METHOD(task_t, process_r, status_t,
	private_ike_natd_t *this, message_t *message)
{
	process_payloads(this, message);

	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_natd_t *this)
{
	return TASK_IKE_NATD;
}

METHOD(task_t, migrate, void,
	private_ike_natd_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->src_seen = FALSE;
	this->dst_seen = FALSE;
	this->src_matched = FALSE;
	this->dst_matched = FALSE;
	this->mapping_changed = FALSE;
	this->fake_no_source_ip = FALSE;
}

METHOD(task_t, destroy, void,
	private_ike_natd_t *this)
{
	DESTROY_IF(this->hasher);
	free(this);
}

METHOD(ike_natd_t, has_mapping_changed, bool,
	private_ike_natd_t *this)
{
	return this->mapping_changed;
}

/*
 * Described in header.
 */
ike_natd_t *ike_natd_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_natd_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.has_mapping_changed = _has_mapping_changed,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1),
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

/*
 * Copyright (C) 2006-2011 Tobias Brunner,
 * Copyright (C) 2006-2007 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
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

/*
 * Copyright (C) 2012 Volker Rümelin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "isakmp_natd.h"

#include <string.h>

#include <hydra.h>
#include <daemon.h>
#include <sa/ikev1/keymat_v1.h>
#include <config/peer_cfg.h>
#include <crypto/hashers/hasher.h>
#include <encoding/payloads/hash_payload.h>

typedef struct private_isakmp_natd_t private_isakmp_natd_t;

/**
 * Private members of a ike_natt_t task.
 */
struct private_isakmp_natd_t {

	/**
	 * Public interface.
	 */
	isakmp_natd_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Keymat derivation (from SA)
	 */
	keymat_v1_t *keymat;

	/**
	 * Did we process any NAT detection payloads for a source address?
	 */
	bool src_seen;

	/**
	 * Did we process any NAT detection payloads for a destination address?
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
};

/**
 * Check if UDP encapsulation has to be forced either by config or required
 * by the kernel interface
 */
static bool force_encap(ike_cfg_t *ike_cfg)
{
	if (!ike_cfg->force_encap(ike_cfg))
	{
		return hydra->kernel_interface->get_features(hydra->kernel_interface) &
					KERNEL_REQUIRE_UDP_ENCAPSULATION;
	}
	return TRUE;
}

/**
 * Get NAT-D payload type (RFC 3947 or RFC 3947 drafts).
 */
static payload_type_t get_nat_d_payload_type(ike_sa_t *ike_sa)
{
	if (ike_sa->supports_extension(ike_sa, EXT_NATT_DRAFT_02_03))
	{
		return PLV1_NAT_D_DRAFT_00_03;
	}
	return PLV1_NAT_D;
}

/**
 * Build NAT detection hash for a host.
 */
static chunk_t generate_natd_hash(private_isakmp_natd_t *this,
								  ike_sa_id_t *ike_sa_id, host_t *host)
{
	hasher_t *hasher;
	chunk_t natd_chunk, natd_hash;
	u_int64_t spi_i, spi_r;
	u_int16_t port;

	hasher = this->keymat->get_hasher(this->keymat);
	if (!hasher)
	{
		DBG1(DBG_IKE, "no hasher available to build NAT-D payload");
		return chunk_empty;
	}

	spi_i = ike_sa_id->get_initiator_spi(ike_sa_id);
	spi_r = ike_sa_id->get_responder_spi(ike_sa_id);
	port = htons(host->get_port(host));

	/*  natd_hash = HASH(CKY-I | CKY-R | IP | Port) */
	natd_chunk = chunk_cata("cccc", chunk_from_thing(spi_i),
						    chunk_from_thing(spi_r), host->get_address(host),
						    chunk_from_thing(port));
	if (!hasher->allocate_hash(hasher, natd_chunk, &natd_hash))
	{
		DBG1(DBG_IKE, "creating NAT-D payload hash failed");
		return chunk_empty;
	}
	DBG3(DBG_IKE, "natd_chunk %B", &natd_chunk);
	DBG3(DBG_IKE, "natd_hash %B", &natd_hash);

	return natd_hash;
}

/**
 * Build a faked NAT-D payload to enforce UDP encapsulation.
 */
static chunk_t generate_natd_hash_faked(private_isakmp_natd_t *this)
{
	hasher_t *hasher;
	chunk_t chunk;
	rng_t *rng;

	hasher = this->keymat->get_hasher(this->keymat);
	if (!hasher)
	{
		DBG1(DBG_IKE, "no hasher available to build NAT-D payload");
		return chunk_empty;
	}
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng ||
		!rng->allocate_bytes(rng, hasher->get_hash_size(hasher), &chunk))
	{
		DBG1(DBG_IKE, "unable to get random bytes for NAT-D fake");
		DESTROY_IF(rng);
		return chunk_empty;
	}
	rng->destroy(rng);
	return chunk;
}

/**
 * Build a NAT-D payload.
 */
static hash_payload_t *build_natd_payload(private_isakmp_natd_t *this, bool src,
										  host_t *host)
{
	hash_payload_t *payload;
	ike_cfg_t *config;
	chunk_t hash;

	config = this->ike_sa->get_ike_cfg(this->ike_sa);
	if (src && force_encap(config))
	{
		hash = generate_natd_hash_faked(this);
	}
	else
	{
		ike_sa_id_t *ike_sa_id = this->ike_sa->get_id(this->ike_sa);
		hash = generate_natd_hash(this, ike_sa_id, host);
	}
	if (!hash.len)
	{
		return NULL;
	}
	payload = hash_payload_create(get_nat_d_payload_type(this->ike_sa));
	payload->set_hash(payload, hash);
	chunk_free(&hash);
	return payload;
}

/**
 * Add NAT-D payloads to the message.
 */
static void add_natd_payloads(private_isakmp_natd_t *this, message_t *message)
{
	hash_payload_t *payload;
	host_t *host;

	/* destination has to be added first */
	host = message->get_destination(message);
	payload = build_natd_payload(this, FALSE, host);
	if (payload)
	{
		message->add_payload(message, (payload_t*)payload);
	}

	/* source is added second, compared with IKEv2 we always know the source,
	 * as these payloads are added in the second Phase 1 exchange or the
	 * response to the first */
	host = message->get_source(message);
	payload = build_natd_payload(this, TRUE, host);
	if (payload)
	{
		message->add_payload(message, (payload_t*)payload);
	}
}

/**
 * Read NAT-D payloads from message and evaluate them.
 */
static void process_payloads(private_isakmp_natd_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	hash_payload_t *hash_payload;
	chunk_t hash, src_hash, dst_hash;
	ike_sa_id_t *ike_sa_id;
	host_t *me, *other;
	ike_cfg_t *config;

	/* precompute hashes for incoming NAT-D comparison */
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
		if (payload->get_type(payload) != PLV1_NAT_D &&
			payload->get_type(payload) != PLV1_NAT_D_DRAFT_00_03)
		{
			continue;
		}
		hash_payload = (hash_payload_t*)payload;
		if (!this->dst_seen)
		{	/* the first NAT-D payload contains the destination hash */
			this->dst_seen = TRUE;
			hash = hash_payload->get_hash(hash_payload);
			DBG3(DBG_IKE, "received dst_hash %B", &hash);
			if (chunk_equals(hash, dst_hash))
			{
				this->dst_matched = TRUE;
			}
			continue;
		}
		/* the other NAT-D payloads contain source hashes */
		this->src_seen = TRUE;
		if (!this->src_matched)
		{
			hash = hash_payload->get_hash(hash_payload);
			DBG3(DBG_IKE, "received src_hash %B", &hash);
			if (chunk_equals(hash, src_hash))
			{
				this->src_matched = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);

	chunk_free(&src_hash);
	chunk_free(&dst_hash);

	if (this->src_seen && this->dst_seen)
	{
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

METHOD(task_t, build_i, status_t,
	private_isakmp_natd_t *this, message_t *message)
{
	status_t result = NEED_MORE;

	switch (message->get_exchange_type(message))
	{
		case AGGRESSIVE:
		{	/* add NAT-D payloads to the second request, already processed
			 * those by the responder contained in the first response */
			result = SUCCESS;
			/* fall */
		}
		case ID_PROT:
		{	/* add NAT-D payloads to the second request, need to process
			 * those by the responder contained in the second response */
			if (message->get_payload(message, PLV1_SECURITY_ASSOCIATION))
			{	/* wait for the second exchange */
				return NEED_MORE;
			}
			add_natd_payloads(this, message);
			return result;
		}
		default:
			break;
	}
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_isakmp_natd_t *this, message_t *message)
{
	status_t result = NEED_MORE;

	if (!this->ike_sa->supports_extension(this->ike_sa, EXT_NATT))
	{	/* we didn't receive VIDs inidcating support for NAT-T */
		return SUCCESS;
	}

	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
		{	/* process NAT-D payloads in the second response, added them in the
			 * second request already, so we're done afterwards */
			if (message->get_payload(message, PLV1_SECURITY_ASSOCIATION))
			{	/* wait for the second exchange */
				return NEED_MORE;
			}
			result = SUCCESS;
			/* fall */
		}
		case AGGRESSIVE:
		{	/* process NAT-D payloads in the first response, add them in the
			 * following second request */
			process_payloads(this, message);

			if (this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY))
			{
				this->ike_sa->float_ports(this->ike_sa);
			}
			return result;
		}
		default:
			break;
	}
	return SUCCESS;
}

METHOD(task_t, process_r, status_t,
	private_isakmp_natd_t *this, message_t *message)
{
	status_t result = NEED_MORE;

	if (!this->ike_sa->supports_extension(this->ike_sa, EXT_NATT))
	{	/* we didn't receive VIDs indicating NAT-T support */
		return SUCCESS;
	}

	switch (message->get_exchange_type(message))
	{
		case AGGRESSIVE:
		{	/* process NAT-D payloads in the second request, already added ours
			 * in the first response */
			result = SUCCESS;
			/* fall */
		}
		case ID_PROT:
		{	/* process NAT-D payloads in the second request, need to add ours
			 * to the second response */
			if (message->get_payload(message, PLV1_SECURITY_ASSOCIATION))
			{	/* wait for the second exchange */
				return NEED_MORE;
			}
			process_payloads(this, message);
			return result;
		}
		default:
			break;
	}
	return SUCCESS;
}

METHOD(task_t, build_r, status_t,
	private_isakmp_natd_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
		{	/* add NAT-D payloads to second response, already processed those
			 * contained in the second request */
			if (message->get_payload(message, PLV1_SECURITY_ASSOCIATION))
			{	/* wait for the second exchange */
				return NEED_MORE;
			}
			add_natd_payloads(this, message);
			return SUCCESS;
		}
		case AGGRESSIVE:
		{	/* add NAT-D payloads to the first response, process those contained
			 * in the following second request */
			add_natd_payloads(this, message);
			return NEED_MORE;
		}
		default:
			break;
	}
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_natd_t *this)
{
	return TASK_ISAKMP_NATD;
}

METHOD(task_t, migrate, void,
	private_isakmp_natd_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
	this->src_seen = FALSE;
	this->dst_seen = FALSE;
	this->src_matched = FALSE;
	this->dst_matched = FALSE;
}

METHOD(task_t, destroy, void,
	private_isakmp_natd_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
isakmp_natd_t *isakmp_natd_create(ike_sa_t *ike_sa, bool initiator)
{
	private_isakmp_natd_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa),
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

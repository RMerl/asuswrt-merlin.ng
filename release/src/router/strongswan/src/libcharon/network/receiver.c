/*
 * Copyright (C) 2008-2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
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

#include <stdlib.h>
#include <unistd.h>

#include "receiver.h"

#include <daemon.h>
#include <network/socket.h>
#include <processing/jobs/job.h>
#include <processing/jobs/process_message_job.h>
#include <processing/jobs/callback_job.h>
#include <crypto/hashers/hasher.h>
#include <threading/mutex.h>
#include <networking/packet.h>

/** lifetime of a cookie, in seconds */
#define COOKIE_LIFETIME 10
/** time we wait before disabling cookies */
#define COOKIE_CALMDOWN_DELAY 10
/** how many times to reuse the secret */
#define COOKIE_REUSE 10000
/** default value for private_receiver_t.cookie_threshold */
#define COOKIE_THRESHOLD_DEFAULT 10
/** default value for private_receiver_t.block_threshold */
#define BLOCK_THRESHOLD_DEFAULT 5
/** length of the secret to use for cookie calculation */
#define SECRET_LENGTH 16
/** Length of a notify payload header */
#define NOTIFY_PAYLOAD_HEADER_LENGTH 8

typedef struct private_receiver_t private_receiver_t;

/**
 * Private data of a receiver_t object.
 */
struct private_receiver_t {
	/**
	 * Public part of a receiver_t object.
	 */
	receiver_t public;

	/**
	 * Registered callback for ESP packets
	 */
	struct {
		receiver_esp_cb_t cb;
		void *data;
	} esp_cb;

	/**
	 * Mutex for ESP callback
	 */
	mutex_t *esp_cb_mutex;

	/**
	 * current secret to use for cookie calculation
	 */
	char secret[SECRET_LENGTH];

	/**
	 * previous secret used to verify older cookies
	 */
	char secret_old[SECRET_LENGTH];

	/**
	 * how many times we have used "secret" so far
	 */
	uint32_t secret_used;

	/**
	 * time we did the cookie switch
	 */
	uint32_t secret_switch;

	/**
	 * time offset to use, hides our system time
	 */
	uint32_t secret_offset;

	/**
	 * the RNG to use for secret generation
	 */
	rng_t *rng;

	/**
	 * hasher to use for cookie calculation
	 */
	hasher_t *hasher;

	/**
	 * require cookies after this many half open IKE_SAs
	 */
	uint32_t cookie_threshold;

	/**
	 * timestamp of last cookie requested
	 */
	time_t last_cookie;

	/**
	 * how many half open IKE_SAs per peer before blocking
	 */
	uint32_t block_threshold;

	/**
	 * Drop IKE_SA_INIT requests if processor job load exceeds this limit
	 */
	u_int init_limit_job_load;

	/**
	 * Drop IKE_SA_INIT requests if half open IKE_SA count exceeds this limit
	 */
	u_int init_limit_half_open;

	/**
	 * Delay for receiving incoming packets, to simulate larger RTT
	 */
	int receive_delay;

	/**
	 * Specific message type to delay, 0 for any
	 */
	int receive_delay_type;

	/**
	 * Delay request messages?
	 */
	bool receive_delay_request;

	/**
	 * Delay response messages?
	 */
	bool receive_delay_response;

	/**
	 * Endpoint is allowed to act as an initiator only
	 */
	bool initiator_only;

};

/**
 * send a notify back to the sender
 */
static void send_notify(message_t *request, int major, exchange_type_t exchange,
						notify_type_t type, chunk_t data)
{
	ike_sa_id_t *ike_sa_id;
	message_t *response;
	host_t *src, *dst;
	packet_t *packet;

	response = message_create(major, 0);
	response->set_exchange_type(response, exchange);
	response->add_notify(response, FALSE, type, data);
	dst = request->get_source(request);
	src = request->get_destination(request);
	response->set_source(response, src->clone(src));
	response->set_destination(response, dst->clone(dst));
	if (major == IKEV2_MAJOR_VERSION)
	{
		response->set_request(response, FALSE);
	}
	response->set_message_id(response, 0);
	ike_sa_id = request->get_ike_sa_id(request);
	ike_sa_id->switch_initiator(ike_sa_id);
	response->set_ike_sa_id(response, ike_sa_id);
	if (response->generate(response, NULL, &packet) == SUCCESS)
	{
		charon->sender->send(charon->sender, packet);
	}
	response->destroy(response);
}

/**
 * build a cookie
 */
static bool cookie_build(private_receiver_t *this, message_t *message,
						 uint32_t t, chunk_t secret, chunk_t *cookie)
{
	uint64_t spi = message->get_initiator_spi(message);
	host_t *ip = message->get_source(message);
	chunk_t input, hash;

	/* COOKIE = t | sha1( IPi | SPIi | t | secret ) */
	input = chunk_cata("cccc", ip->get_address(ip), chunk_from_thing(spi),
					  chunk_from_thing(t), secret);
	hash = chunk_alloca(this->hasher->get_hash_size(this->hasher));
	if (!this->hasher->get_hash(this->hasher, input, hash.ptr))
	{
		return FALSE;
	}
	*cookie = chunk_cat("cc", chunk_from_thing(t), hash);
	return TRUE;
}

/**
 * verify a received cookie
 */
static bool cookie_verify(private_receiver_t *this, message_t *message,
						  chunk_t cookie)
{
	uint32_t t, now;
	chunk_t reference;
	chunk_t secret;

	now = time_monotonic(NULL);
	t = *(uint32_t*)cookie.ptr;

	if (cookie.len != sizeof(uint32_t) +
			this->hasher->get_hash_size(this->hasher) ||
		t < now - this->secret_offset - COOKIE_LIFETIME)
	{
		DBG2(DBG_NET, "received cookie lifetime expired, rejecting");
		return FALSE;
	}

	/* check if cookie is derived from old_secret */
	if (t + this->secret_offset > this->secret_switch)
	{
		secret = chunk_from_thing(this->secret);
	}
	else
	{
		secret = chunk_from_thing(this->secret_old);
	}

	/* compare own calculation against received */
	if (!cookie_build(this, message, t, secret, &reference))
	{
		return FALSE;
	}
	if (chunk_equals_const(reference, cookie))
	{
		chunk_free(&reference);
		return TRUE;
	}
	chunk_free(&reference);
	return FALSE;
}

/**
 * Check if a valid cookie found
 */
static bool check_cookie(private_receiver_t *this, message_t *message)
{
	chunk_t data;

	/* check for a cookie. We don't use our parser here and do it
	 * quick and dirty for performance reasons.
	 * we assume the cookie is the first payload (which is a MUST), and
	 * the cookie's SPI length is zero. */
	data = message->get_packet_data(message);
	if (data.len <
		 IKE_HEADER_LENGTH + NOTIFY_PAYLOAD_HEADER_LENGTH +
		 sizeof(uint32_t) + this->hasher->get_hash_size(this->hasher) ||
		*(data.ptr + 16) != PLV2_NOTIFY ||
		*(uint16_t*)(data.ptr + IKE_HEADER_LENGTH + 6) != htons(COOKIE))
	{
		/* no cookie found */
		return FALSE;
	}
	data.ptr += IKE_HEADER_LENGTH + NOTIFY_PAYLOAD_HEADER_LENGTH;
	data.len = sizeof(uint32_t) + this->hasher->get_hash_size(this->hasher);
	if (!cookie_verify(this, message, data))
	{
		DBG2(DBG_NET, "found cookie, but content invalid");
		return FALSE;
	}
	return TRUE;
}

/**
 * Check if we currently require cookies
 */
static bool cookie_required(private_receiver_t *this,
							u_int half_open, uint32_t now)
{
	if (this->cookie_threshold && half_open >= this->cookie_threshold)
	{
		this->last_cookie = now;
		return TRUE;
	}
	if (this->last_cookie && now < this->last_cookie + COOKIE_CALMDOWN_DELAY)
	{
		/* We don't disable cookies unless we haven't seen IKE_SA_INITs
		 * for COOKIE_CALMDOWN_DELAY seconds. This avoids jittering between
		 * cookie on / cookie off states, which is problematic. Consider the
		 * following: A legitimiate initiator sends a IKE_SA_INIT while we
		 * are under a DoS attack. If we toggle our cookie behavior,
		 * multiple retransmits of this IKE_SA_INIT might get answered with
		 * and without cookies. The initiator goes on and retries with
		 * a cookie, but it can't know if the completing IKE_SA_INIT response
		 * is to its IKE_SA_INIT request with or without cookies. This is
		 * problematic, as the cookie is part of AUTH payload data.
		 */
		this->last_cookie = now;
		return TRUE;
	}
	return FALSE;
}

/**
 * Check if we should drop IKE_SA_INIT because of cookie/overload checking
 */
static bool drop_ike_sa_init(private_receiver_t *this, message_t *message)
{
	u_int half_open;
	uint32_t now;

	now = time_monotonic(NULL);
	half_open = charon->ike_sa_manager->get_half_open_count(
										charon->ike_sa_manager, NULL, TRUE);

	/* check for cookies in IKEv2 */
	if (message->get_major_version(message) == IKEV2_MAJOR_VERSION &&
		cookie_required(this, half_open, now) && !check_cookie(this, message))
	{
		chunk_t cookie;

		DBG2(DBG_NET, "received packet from: %#H to %#H",
			 message->get_source(message),
			 message->get_destination(message));
		if (!cookie_build(this, message, now - this->secret_offset,
						  chunk_from_thing(this->secret), &cookie))
		{
			return TRUE;
		}
		DBG2(DBG_NET, "sending COOKIE notify to %H",
			 message->get_source(message));
		send_notify(message, IKEV2_MAJOR_VERSION, IKE_SA_INIT, COOKIE, cookie);
		chunk_free(&cookie);
		if (++this->secret_used > COOKIE_REUSE)
		{
			char secret[SECRET_LENGTH];

			DBG1(DBG_NET, "generating new cookie secret after %d uses",
				 this->secret_used);
			if (this->rng->get_bytes(this->rng, SECRET_LENGTH, secret))
			{
				memcpy(this->secret_old, this->secret, SECRET_LENGTH);
				memcpy(this->secret, secret, SECRET_LENGTH);
				memwipe(secret, SECRET_LENGTH);
				this->secret_switch = now;
				this->secret_used = 0;
			}
			else
			{
				DBG1(DBG_NET, "failed to allocated cookie secret, keeping old");
			}
		}
		return TRUE;
	}

	/* check if peer has too many IKE_SAs half open */
	if (this->block_threshold &&
		charon->ike_sa_manager->get_half_open_count(charon->ike_sa_manager,
				message->get_source(message), TRUE) >= this->block_threshold)
	{
		DBG1(DBG_NET, "ignoring IKE_SA setup from %H, "
			 "peer too aggressive", message->get_source(message));
		return TRUE;
	}

	/* check if global half open IKE_SA limit reached */
	if (this->init_limit_half_open &&
	    half_open >= this->init_limit_half_open)
	{
		DBG1(DBG_NET, "ignoring IKE_SA setup from %H, half open IKE_SA "
			 "count of %d exceeds limit of %d", message->get_source(message),
			 half_open, this->init_limit_half_open);
		return TRUE;
	}

	/* check if job load acceptable */
	if (this->init_limit_job_load)
	{
		u_int jobs = 0, i;

		for (i = 0; i < JOB_PRIO_MAX; i++)
		{
			jobs += lib->processor->get_job_load(lib->processor, i);
		}
		if (jobs > this->init_limit_job_load)
		{
			DBG1(DBG_NET, "ignoring IKE_SA setup from %H, job load of %d "
				 "exceeds limit of %d", message->get_source(message),
				 jobs, this->init_limit_job_load);
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Job callback to receive packets
 */
static job_requeue_t receive_packets(private_receiver_t *this)
{
	ike_sa_id_t *id;
	packet_t *packet;
	message_t *message;
	host_t *src, *dst;
	status_t status;
	bool supported = TRUE;
	chunk_t data, marker = chunk_from_chars(0x00, 0x00, 0x00, 0x00);

	/* read in a packet */
	status = charon->socket->receive(charon->socket, &packet);
	if (status == NOT_SUPPORTED)
	{
		return JOB_REQUEUE_NONE;
	}
	else if (status != SUCCESS)
	{
		DBG2(DBG_NET, "receiving from socket failed!");
		return JOB_REQUEUE_FAIR;
	}

	data = packet->get_data(packet);
	if (data.len == 1 && data.ptr[0] == 0xFF)
	{	/* silently drop NAT-T keepalives */
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}
	else if (data.len < marker.len)
	{	/* drop packets that are too small */
		DBG3(DBG_NET, "received packet is too short (%d bytes)", data.len);
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}

	dst = packet->get_destination(packet);
	src = packet->get_source(packet);
	if (!charon->kernel->all_interfaces_usable(charon->kernel)
		&& !charon->kernel->get_interface(charon->kernel, dst, NULL))
	{
		DBG3(DBG_NET, "received packet from %#H to %#H on ignored interface",
			 src, dst);
		packet->destroy(packet);
		return JOB_REQUEUE_DIRECT;
	}

	/* if neither source nor destination port is 500 we assume an IKE packet
	 * with Non-ESP marker or an ESP packet */
	if (dst->get_port(dst) != IKEV2_UDP_PORT &&
		src->get_port(src) != IKEV2_UDP_PORT)
	{
		if (memeq(data.ptr, marker.ptr, marker.len))
		{	/* remove Non-ESP marker */
			packet->skip_bytes(packet, marker.len);
		}
		else
		{	/* this seems to be an ESP packet */
			this->esp_cb_mutex->lock(this->esp_cb_mutex);
			if (this->esp_cb.cb)
			{
				this->esp_cb.cb(this->esp_cb.data, packet);
			}
			else
			{
				packet->destroy(packet);
			}
			this->esp_cb_mutex->unlock(this->esp_cb_mutex);
			return JOB_REQUEUE_DIRECT;
		}
	}

	/* parse message header */
	message = message_create_from_packet(packet);
	if (message->parse_header(message) != SUCCESS)
	{
		DBG1(DBG_NET, "received invalid IKE header from %H - ignored",
			 packet->get_source(packet));
		charon->bus->alert(charon->bus, ALERT_PARSE_ERROR_HEADER, message);
		message->destroy(message);
		return JOB_REQUEUE_DIRECT;
	}

	/* check IKE major version */
	switch (message->get_major_version(message))
	{
		case IKEV2_MAJOR_VERSION:
#ifndef USE_IKEV2
			if (message->get_exchange_type(message) == IKE_SA_INIT &&
				message->get_request(message))
			{
				send_notify(message, IKEV1_MAJOR_VERSION, INFORMATIONAL_V1,
							INVALID_MAJOR_VERSION, chunk_empty);
				supported = FALSE;
			}
#endif /* USE_IKEV2 */
			break;
		case IKEV1_MAJOR_VERSION:
#ifndef USE_IKEV1
			if (message->get_exchange_type(message) == ID_PROT ||
				message->get_exchange_type(message) == AGGRESSIVE)
			{
				send_notify(message, IKEV2_MAJOR_VERSION, INFORMATIONAL,
							INVALID_MAJOR_VERSION, chunk_empty);
				supported = FALSE;
			}
#endif /* USE_IKEV1 */
			break;
		default:
#ifdef USE_IKEV2
			send_notify(message, IKEV2_MAJOR_VERSION, INFORMATIONAL,
						INVALID_MAJOR_VERSION, chunk_empty);
#elif defined(USE_IKEV1)
			send_notify(message, IKEV1_MAJOR_VERSION, INFORMATIONAL_V1,
						INVALID_MAJOR_VERSION, chunk_empty);
#endif /* USE_IKEV1 */
			supported = FALSE;
			break;
	}
	if (!supported)
	{
		DBG1(DBG_NET, "received unsupported IKE version %d.%d from %H, sending "
			 "INVALID_MAJOR_VERSION", message->get_major_version(message),
			 message->get_minor_version(message), packet->get_source(packet));
		message->destroy(message);
		return JOB_REQUEUE_DIRECT;
	}
	if (message->get_request(message) &&
		message->get_exchange_type(message) == IKE_SA_INIT)
	{
		id = message->get_ike_sa_id(message);
		if (this->initiator_only || !id->is_initiator(id) ||
			drop_ike_sa_init(this, message))
		{
			message->destroy(message);
			return JOB_REQUEUE_DIRECT;
		}
	}
	if (message->get_exchange_type(message) == ID_PROT ||
		message->get_exchange_type(message) == AGGRESSIVE)
	{
		id = message->get_ike_sa_id(message);
		if (id->get_responder_spi(id) == 0 &&
		   (this->initiator_only || drop_ike_sa_init(this, message)))
		{
			message->destroy(message);
			return JOB_REQUEUE_DIRECT;
		}
	}

	if (this->receive_delay)
	{
		if (this->receive_delay_type == 0 ||
			this->receive_delay_type == message->get_exchange_type(message))
		{
			if ((message->get_request(message) && this->receive_delay_request) ||
				(!message->get_request(message) && this->receive_delay_response))
			{
				DBG1(DBG_NET, "using receive delay: %dms",
					 this->receive_delay);
				lib->scheduler->schedule_job_ms(lib->scheduler,
								(job_t*)process_message_job_create(message),
								this->receive_delay);
				return JOB_REQUEUE_DIRECT;
			}
		}
	}
	lib->processor->queue_job(lib->processor,
							  (job_t*)process_message_job_create(message));
	return JOB_REQUEUE_DIRECT;
}

METHOD(receiver_t, add_esp_cb, void,
	private_receiver_t *this, receiver_esp_cb_t callback, void *data)
{
	this->esp_cb_mutex->lock(this->esp_cb_mutex);
	this->esp_cb.cb = callback;
	this->esp_cb.data = data;
	this->esp_cb_mutex->unlock(this->esp_cb_mutex);
}

METHOD(receiver_t, del_esp_cb, void,
	private_receiver_t *this, receiver_esp_cb_t callback)
{
	this->esp_cb_mutex->lock(this->esp_cb_mutex);
	if (this->esp_cb.cb == callback)
	{
		this->esp_cb.cb = NULL;
		this->esp_cb.data = NULL;
	}
	this->esp_cb_mutex->unlock(this->esp_cb_mutex);
}

METHOD(receiver_t, destroy, void,
	private_receiver_t *this)
{
	this->rng->destroy(this->rng);
	this->hasher->destroy(this->hasher);
	this->esp_cb_mutex->destroy(this->esp_cb_mutex);
	free(this);
}

/*
 * Described in header.
 */
receiver_t *receiver_create()
{
	private_receiver_t *this;
	uint32_t now = time_monotonic(NULL);

	INIT(this,
		.public = {
			.add_esp_cb = _add_esp_cb,
			.del_esp_cb = _del_esp_cb,
			.destroy = _destroy,
		},
		.esp_cb_mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.secret_switch = now,
		.secret_offset = random() % now,
	);

	if (lib->settings->get_bool(lib->settings,
								"%s.dos_protection", TRUE, lib->ns))
	{
		this->cookie_threshold = lib->settings->get_int(lib->settings,
					"%s.cookie_threshold", COOKIE_THRESHOLD_DEFAULT, lib->ns);
		this->block_threshold = lib->settings->get_int(lib->settings,
					"%s.block_threshold", BLOCK_THRESHOLD_DEFAULT, lib->ns);
	}
	this->init_limit_job_load = lib->settings->get_int(lib->settings,
					"%s.init_limit_job_load", 0, lib->ns);
	this->init_limit_half_open = lib->settings->get_int(lib->settings,
					"%s.init_limit_half_open", 0, lib->ns);
	this->receive_delay = lib->settings->get_int(lib->settings,
					"%s.receive_delay", 0, lib->ns);
	this->receive_delay_type = lib->settings->get_int(lib->settings,
					"%s.receive_delay_type", 0, lib->ns);
	this->receive_delay_request = lib->settings->get_bool(lib->settings,
					"%s.receive_delay_request", TRUE, lib->ns);
	this->receive_delay_response = lib->settings->get_bool(lib->settings,
					"%s.receive_delay_response", TRUE, lib->ns);
	this->initiator_only = lib->settings->get_bool(lib->settings,
					"%s.initiator_only", FALSE, lib->ns);

	this->hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!this->hasher)
	{
		DBG1(DBG_NET, "creating cookie hasher failed, no hashers supported");
		free(this);
		return NULL;
	}
	this->rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!this->rng)
	{
		DBG1(DBG_NET, "creating cookie RNG failed, no RNG supported");
		this->hasher->destroy(this->hasher);
		free(this);
		return NULL;
	}
	if (!this->rng->get_bytes(this->rng, SECRET_LENGTH, this->secret))
	{
		DBG1(DBG_NET, "creating cookie secret failed");
		destroy(this);
		return NULL;
	}
	memcpy(this->secret_old, this->secret, SECRET_LENGTH);

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)receive_packets,
			this, NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));

	return &this->public;
}

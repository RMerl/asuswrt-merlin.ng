/*
 * Copyright (C) 2007 Martin Willi
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

#include "eap_md5.h"

#include <daemon.h>
#include <library.h>
#include <crypto/hashers/hasher.h>

typedef struct private_eap_md5_t private_eap_md5_t;

/**
 * Private data of an eap_md5_t object.
 */
struct private_eap_md5_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_md5_t public;

	/**
	 * ID of the server
	 */
	identification_t *server;

	/**
	 * ID of the peer
	 */
	identification_t *peer;

	/**
	 * challenge sent by the server
	 */
	chunk_t challenge;

	/**
	 * EAP message identifier
	 */
	u_int8_t identifier;
};

typedef struct eap_md5_header_t eap_md5_header_t;

/**
 * packed eap MD5 header struct
 */
struct eap_md5_header_t {
	/** EAP code (REQUEST/RESPONSE) */
	u_int8_t code;
	/** unique message identifier */
	u_int8_t identifier;
	/** length of whole message */
	u_int16_t length;
	/** EAP type */
	u_int8_t type;
	/** length of value (challenge) */
	u_int8_t value_size;
	/** actual value */
	u_int8_t value[];
} __attribute__((__packed__));

#define CHALLENGE_LEN 16
#define PAYLOAD_LEN (CHALLENGE_LEN + sizeof(eap_md5_header_t))

/**
 * Hash the challenge string, create response
 */
static status_t hash_challenge(private_eap_md5_t *this, chunk_t *response,
							   identification_t *me, identification_t *other)
{
	shared_key_t *shared;
	chunk_t concat;
	hasher_t *hasher;

	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP, me, other);
	if (shared == NULL)
	{
		DBG1(DBG_IKE, "no EAP key found for hosts '%Y' - '%Y'", me, other);
		return NOT_FOUND;
	}
	concat = chunk_cata("ccc", chunk_from_thing(this->identifier),
						shared->get_key(shared), this->challenge);
	shared->destroy(shared);
	hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5);
	if (hasher == NULL)
	{
		DBG1(DBG_IKE, "EAP-MD5 failed, MD5 not supported");
		return FAILED;
	}
	if (!hasher->allocate_hash(hasher, concat, response))
	{
		hasher->destroy(hasher);
		return FAILED;
	}
	hasher->destroy(hasher);
	return SUCCESS;
}

METHOD(eap_method_t, initiate_peer, status_t,
	private_eap_md5_t *this, eap_payload_t **out)
{
	/* peer never initiates */
	return FAILED;
}

METHOD(eap_method_t, initiate_server, status_t,
	private_eap_md5_t *this, eap_payload_t **out)
{
	rng_t *rng;
	eap_md5_header_t *req;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->allocate_bytes(rng, CHALLENGE_LEN, &this->challenge))
	{
		DESTROY_IF(rng);
		return FAILED;
	}
	rng->destroy(rng);

	req = alloca(PAYLOAD_LEN);
	req->length = htons(PAYLOAD_LEN);
	req->code = EAP_REQUEST;
	req->identifier = this->identifier;
	req->type = EAP_MD5;
	req->value_size = this->challenge.len;
	memcpy(req->value, this->challenge.ptr, this->challenge.len);

	*out = eap_payload_create_data(chunk_create((void*)req, PAYLOAD_LEN));
	return NEED_MORE;
}

METHOD(eap_method_t, process_peer, status_t,
	private_eap_md5_t *this, eap_payload_t *in, eap_payload_t **out)
{
	chunk_t response;
	chunk_t data;
	eap_md5_header_t *req;

	this->identifier = in->get_identifier(in);
	data = in->get_data(in);
	if (data.len < 6 || data.ptr[5] + 6 > data.len)
	{
		DBG1(DBG_IKE, "received invalid EAP-MD5 message");
		return FAILED;
	}
	this->challenge = chunk_clone(chunk_create(data.ptr + 6, data.ptr[5]));
	if (hash_challenge(this, &response, this->peer, this->server) != SUCCESS)
	{
		return FAILED;
	}
	req = alloca(PAYLOAD_LEN);
	req->length = htons(PAYLOAD_LEN);
	req->code = EAP_RESPONSE;
	req->identifier = this->identifier;
	req->type = EAP_MD5;
	req->value_size = response.len;
	memcpy(req->value, response.ptr, response.len);
	chunk_free(&response);

	*out = eap_payload_create_data(chunk_create((void*)req, PAYLOAD_LEN));
	return NEED_MORE;
}

METHOD(eap_method_t, process_server, status_t,
	private_eap_md5_t *this, eap_payload_t *in, eap_payload_t **out)
{
	chunk_t response, expected;
	chunk_t data;

	data = in->get_data(in);
	if (this->identifier != in->get_identifier(in) ||
		data.len < 6 || data.ptr[5] + 6 > data.len)
	{
		DBG1(DBG_IKE, "received invalid EAP-MD5 message");
		return FAILED;
	}
	if (hash_challenge(this, &expected, this->server, this->peer) != SUCCESS)
	{
		return FAILED;
	}
	response = chunk_create(data.ptr + 6, data.ptr[5]);
	if (response.len < expected.len ||
		!memeq(response.ptr, expected.ptr, expected.len))
	{
		chunk_free(&expected);
		DBG1(DBG_IKE, "EAP-MD5 verification failed");
		return FAILED;
	}
	chunk_free(&expected);
	return SUCCESS;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_md5_t *this, u_int32_t *vendor)
{
	*vendor = 0;
	return EAP_MD5;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_md5_t *this, chunk_t *msk)
{
	return FAILED;
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_md5_t *this)
{
	return FALSE;
}

METHOD(eap_method_t, get_identifier, u_int8_t,
	private_eap_md5_t *this)
{
	return this->identifier;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_md5_t *this, u_int8_t identifier)
{
	this->identifier = identifier;
}

METHOD(eap_method_t, destroy, void,
	private_eap_md5_t *this)
{
	this->peer->destroy(this->peer);
	this->server->destroy(this->server);
	chunk_free(&this->challenge);
	free(this);
}

/*
 * See header
 */
eap_md5_t *eap_md5_create_server(identification_t *server, identification_t *peer)
{
	private_eap_md5_t *this;

	INIT(this,
		.public = {
			.eap_method = {
				.initiate = _initiate_server,
				.process = _process_server,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
		.server = server->clone(server),
	);

	/* generate a non-zero identifier */
	do {
		this->identifier = random();
	} while (!this->identifier);

	return &this->public;
}

/*
 * See header
 */
eap_md5_t *eap_md5_create_peer(identification_t *server, identification_t *peer)
{
	private_eap_md5_t *this;

	INIT(this,
		.public = {
			.eap_method = {
				.initiate = _initiate_peer,
				.process = _process_peer,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
		.server = server->clone(server),
	);

	return &this->public;
}


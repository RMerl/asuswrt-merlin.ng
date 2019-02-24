/*
 * Copyright (C) 2007-2008 Martin Willi
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

#include "eap_identity.h"

#include <daemon.h>
#include <library.h>

typedef struct private_eap_identity_t private_eap_identity_t;

/**
 * Private data of an eap_identity_t object.
 */
struct private_eap_identity_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_identity_t public;

	/**
	 * ID of the peer
	 */
	identification_t *peer;

	/**
	 * received identity chunk
	 */
	chunk_t identity;

	/**
	 * EAP identifier
	 */
	uint8_t identifier;
};

typedef struct eap_identity_header_t eap_identity_header_t;

/**
 * packed EAP Identity header struct
 */
struct eap_identity_header_t {
	/** EAP code (REQUEST/RESPONSE) */
	uint8_t code;
	/** unique message identifier */
	uint8_t identifier;
	/** length of whole message */
	uint16_t length;
	/** EAP type */
	uint8_t type;
	/** identity data */
	uint8_t data[];
} __attribute__((__packed__));

METHOD(eap_method_t, process_peer, status_t,
	private_eap_identity_t *this, eap_payload_t *in, eap_payload_t **out)
{
	chunk_t id;
	eap_identity_header_t *hdr;
	size_t len;

	id = this->peer->get_encoding(this->peer);
	len = sizeof(eap_identity_header_t) + id.len;
	if (in)
	{
		this->identifier = in->get_identifier(in);
	}
	hdr = alloca(len);
	hdr->code = EAP_RESPONSE;
	hdr->identifier = this->identifier;
	hdr->length = htons(len);
	hdr->type = EAP_IDENTITY;
	memcpy(hdr->data, id.ptr, id.len);

	*out = eap_payload_create_data(chunk_create((u_char*)hdr, len));
	return SUCCESS;
}

METHOD(eap_method_t, initiate_peer, status_t,
	private_eap_identity_t *this, eap_payload_t **out)
{
	/* peer never initiates */
	return FAILED;
}

METHOD(eap_method_t, process_server, status_t,
	private_eap_identity_t *this, eap_payload_t *in, eap_payload_t **out)
{
	chunk_t data;

	data = chunk_skip(in->get_data(in), 5);
	if (data.len)
	{
		this->identity = chunk_clone(data);
	}
	return SUCCESS;
}

METHOD(eap_method_t, initiate_server, status_t,
	private_eap_identity_t *this, eap_payload_t **out)
{
	eap_identity_header_t hdr;

	hdr.code = EAP_REQUEST;
	hdr.identifier = this->identifier;
	hdr.length = htons(sizeof(eap_identity_header_t));
	hdr.type = EAP_IDENTITY;

	*out = eap_payload_create_data(chunk_create((u_char*)&hdr,
												sizeof(eap_identity_header_t)));
	return NEED_MORE;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_identity_t *this, uint32_t *vendor)
{
	*vendor = 0;
	return EAP_IDENTITY;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_identity_t *this, chunk_t *msk)
{
	if (this->identity.ptr)
	{
		*msk = this->identity;
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_identity_t *this)
{
	return this->identifier;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_identity_t *this, uint8_t identifier)
{
	this->identifier = identifier;
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_identity_t *this)
{
	return FALSE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_identity_t *this)
{
	this->peer->destroy(this->peer);
	free(this->identity.ptr);
	free(this);
}

/*
 * Described in header.
 */
eap_identity_t *eap_identity_create_peer(identification_t *server,
										 identification_t *peer)
{
	private_eap_identity_t *this;

	INIT(this,
		.public =  {
			.eap_method = {
				.initiate = _initiate_peer,
				.process = _process_peer,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
		.peer = peer->clone(peer),
		.identity = chunk_empty,
	);

	return &this->public;
}

/*
 * Described in header.
 */
eap_identity_t *eap_identity_create_server(identification_t *server,
										   identification_t *peer)
{
	private_eap_identity_t *this;

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
		.identity = chunk_empty,
	);

	return &this->public;
}


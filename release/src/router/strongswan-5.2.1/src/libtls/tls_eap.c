
/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "tls_eap.h"

#include "tls.h"

#include <utils/debug.h>
#include <library.h>

/**
 * Size limit for a TLS message allowing for worst-case protection overhead
 * according to section 6.2.3. "Payload Protection" of RFC 5246 TLS 1.2
 */
#define TLS_MAX_MESSAGE_LEN		4 * (TLS_MAX_FRAGMENT_LEN + 2048)

typedef struct private_tls_eap_t private_tls_eap_t;

/**
 * Private data of an tls_eap_t object.
 */
struct private_tls_eap_t {

	/**
	 * Public tls_eap_t interface.
	 */
	tls_eap_t public;

	/**
	 * Type of EAP method, EAP-TLS, EAP-TTLS, or EAP-TNC
	 */
	eap_type_t type;

	/**
	 * Current value of EAP identifier
	 */
	uint8_t identifier;

	/**
	 * TLS stack
	 */
	tls_t *tls;

	/**
	 * Role
	 */
	bool is_server;

	/**
	 * Supported version of the EAP tunnel protocol
	 */
	uint8_t supported_version;

	/**
	 * If FALSE include the total length of an EAP message
	 * in the first fragment of fragmented messages only.
	 * If TRUE also include the length in non-fragmented messages.
	 */
	bool include_length;

	/**
	 * First fragment of a multi-fragment record?
	 */
	bool first_fragment;

	/**
	 * Maximum size of an outgoing EAP-TLS fragment
	 */
	size_t frag_size;

	/**
	 * Number of EAP messages/fragments processed so far
	 */
	int processed;

	/**
	 * Maximum number of processed EAP messages/fragments
	 */
	int max_msg_count;
};

/**
 * Flags of an EAP-TLS/TTLS/TNC message
 */
typedef enum {
	EAP_TLS_LENGTH = (1<<7),		/* shared with EAP-TTLS/TNC/PEAP */
	EAP_TLS_MORE_FRAGS = (1<<6),	/* shared with EAP-TTLS/TNC/PEAP */
	EAP_TLS_START = (1<<5),			/* shared with EAP-TTLS/TNC/PEAP */
	EAP_TTLS_VERSION = (0x07),		/* shared with EAP-TNC/PEAP/PT-EAP */
	EAP_PT_START = (1<<7)			/* PT-EAP only */
} eap_tls_flags_t;

#define EAP_TTLS_SUPPORTED_VERSION		0
#define EAP_TNC_SUPPORTED_VERSION		1
#define EAP_PEAP_SUPPORTED_VERSION		0
#define EAP_PT_EAP_SUPPORTED_VERSION	1

/**
 * EAP-TLS/TTLS packet format
 */
typedef struct __attribute__((packed)) {
	uint8_t code;
	uint8_t identifier;
	uint16_t length;
	uint8_t type;
	uint8_t flags;
} eap_tls_packet_t;

METHOD(tls_eap_t, initiate, status_t,
	private_tls_eap_t *this, chunk_t *out)
{
	if (this->is_server)
	{
		eap_tls_packet_t pkt = {
			.type = this->type,
			.code = EAP_REQUEST,
			.flags = this->supported_version
		};
		switch (this->type)
		{
			case EAP_TLS:
			case EAP_TTLS:
			case EAP_TNC:
			case EAP_PEAP:
				pkt.flags |= EAP_TLS_START;
				break;
			case EAP_PT_EAP:
				pkt.flags |= EAP_PT_START;
				break;
			default:
				break;
		}
		htoun16(&pkt.length, sizeof(eap_tls_packet_t));
		pkt.identifier = this->identifier;

		*out = chunk_clone(chunk_from_thing(pkt));
		DBG2(DBG_TLS, "sending %N start packet (%u bytes)",
			 eap_type_names, this->type, sizeof(eap_tls_packet_t));
		DBG3(DBG_TLS, "%B", out);
		return NEED_MORE;
	}
	return FAILED;
}

/**
 * Process a received packet
 */
static status_t process_pkt(private_tls_eap_t *this, eap_tls_packet_t *pkt)
{
	uint8_t version;
	uint16_t pkt_len;
	uint32_t msg_len;
	size_t msg_len_offset = 0;

	/* EAP-TLS doesn't have a version field */
	if (this->type != EAP_TLS)
	{
		version = pkt->flags & EAP_TTLS_VERSION;
		if (version != this->supported_version)
		{
			DBG1(DBG_TLS, "received %N packet with unsupported version v%u",
			eap_type_names, this->type, version);
			return FAILED;
		}
	}
	pkt_len = untoh16(&pkt->length);

	if (this->type != EAP_PT_EAP && (pkt->flags & EAP_TLS_LENGTH))
	{
		if (pkt_len < sizeof(eap_tls_packet_t) + sizeof(msg_len))
		{
			DBG1(DBG_TLS, "%N packet too short", eap_type_names, this->type);
			return FAILED;
		}
		msg_len = untoh32(pkt + 1);
		if (msg_len < pkt_len - sizeof(eap_tls_packet_t) - sizeof(msg_len) ||
			msg_len > TLS_MAX_MESSAGE_LEN)
		{
			DBG1(DBG_TLS, "invalid %N packet length (%u bytes)", eap_type_names,
				 this->type, msg_len);
			return FAILED;
		}
		msg_len_offset = sizeof(msg_len);
	}

	return this->tls->process(this->tls, (char*)(pkt + 1) + msg_len_offset,
					   pkt_len - sizeof(eap_tls_packet_t) - msg_len_offset);
}

/**
 * Build a packet to send
 */
static status_t build_pkt(private_tls_eap_t *this, chunk_t *out)
{
	char buf[this->frag_size];
	eap_tls_packet_t *pkt;
	size_t len, reclen, msg_len_offset;
	status_t status;
	char *kind;

	if (this->is_server)
	{
		this->identifier++;
	}
	pkt = (eap_tls_packet_t*)buf;
	pkt->code = this->is_server ? EAP_REQUEST : EAP_RESPONSE;
	pkt->identifier = this->identifier;
	pkt->type = this->type;
	pkt->flags = this->supported_version;

	if (this->first_fragment)
	{
		len = sizeof(buf) - sizeof(eap_tls_packet_t) - sizeof(uint32_t);
		msg_len_offset = sizeof(uint32_t);
	}
	else
	{
		len = sizeof(buf) - sizeof(eap_tls_packet_t);
		msg_len_offset = 0;
	}
	status = this->tls->build(this->tls, buf + sizeof(eap_tls_packet_t) +
										 msg_len_offset, &len, &reclen);

	switch (status)
	{
		case NEED_MORE:
			pkt->flags |= EAP_TLS_MORE_FRAGS;
			kind = "further fragment";
			if (this->first_fragment)
			{
				pkt->flags |= EAP_TLS_LENGTH;
				this->first_fragment = FALSE;
				kind = "first fragment";
			}
			break;
		case ALREADY_DONE:
			if (this->first_fragment)
			{
				if (this->include_length)
				{
					pkt->flags |= EAP_TLS_LENGTH;
				}
				kind = "packet";
			}
			else if (this->type != EAP_TNC && this->type != EAP_PT_EAP)
			{
				this->first_fragment = TRUE;
				kind = "final fragment";
			}
			else
			{
				kind = "packet";
			}
			break;
		default:
			return status;
	}
	if (reclen)
	{
		if (pkt->flags & EAP_TLS_LENGTH)
		{
			htoun32(pkt + 1, reclen);
			len += sizeof(uint32_t);
			pkt->flags |= EAP_TLS_LENGTH;
		}
		else
		{
			/* get rid of the reserved length field */
			memmove(buf + sizeof(eap_tls_packet_t),
					buf + sizeof(eap_tls_packet_t) + sizeof(uint32_t), len);
		}
	}
	len += sizeof(eap_tls_packet_t);
	htoun16(&pkt->length, len);
	*out = chunk_clone(chunk_create(buf, len));
	DBG2(DBG_TLS, "sending %N %s (%u bytes)",
		 eap_type_names, this->type, kind, len);
	DBG3(DBG_TLS, "%B", out);
	return NEED_MORE;
}

/**
 * Send an ack to request next fragment
 */
static chunk_t create_ack(private_tls_eap_t *this)
{
	eap_tls_packet_t pkt = {
		.code = this->is_server ? EAP_REQUEST : EAP_RESPONSE,
		.type = this->type,
	};

	if (this->is_server)
	{
		this->identifier++;
	}
	pkt.identifier = this->identifier;
	htoun16(&pkt.length, sizeof(pkt));

	switch (this->type)
	{
		case EAP_TTLS:
			pkt.flags |= EAP_TTLS_SUPPORTED_VERSION;
			break;
		case EAP_TNC:
			pkt.flags |= EAP_TNC_SUPPORTED_VERSION;
			break;
		case EAP_PEAP:
			pkt.flags |= EAP_PEAP_SUPPORTED_VERSION;
			break;
		default:
			break;
	}
	DBG2(DBG_TLS, "sending %N acknowledgement packet",
		 eap_type_names, this->type);
	return chunk_clone(chunk_from_thing(pkt));
}

METHOD(tls_eap_t, process, status_t,
	private_tls_eap_t *this, chunk_t in, chunk_t *out)
{
	eap_tls_packet_t *pkt;
	status_t status;

	if (this->max_msg_count && ++this->processed > this->max_msg_count)
	{
		DBG1(DBG_TLS, "%N packet count exceeded (%d > %d)",
			 eap_type_names, this->type,
			 this->processed, this->max_msg_count);
		return FAILED;
	}

	pkt = (eap_tls_packet_t*)in.ptr;
	if (in.len < sizeof(eap_tls_packet_t) || untoh16(&pkt->length) != in.len)
	{
		DBG1(DBG_TLS, "invalid %N packet length", eap_type_names, this->type);
		return FAILED;
	}

	/* update EAP identifier */
	if (!this->is_server)
	{
		this->identifier = pkt->identifier;
	}
	DBG3(DBG_TLS, "%N payload %B", eap_type_names, this->type, &in);

	if ((this->type == EAP_PT_EAP && (pkt->flags & EAP_PT_START)) ||
        (pkt->flags & EAP_TLS_START))
	{
		if (this->type == EAP_TTLS || this->type == EAP_TNC ||
			this->type == EAP_PEAP || this->type == EAP_PT_EAP)
		{
			DBG1(DBG_TLS, "%N version is v%u", eap_type_names, this->type,
				 pkt->flags & EAP_TTLS_VERSION);
		}
	}
	else
	{
		if (in.len == sizeof(eap_tls_packet_t))
		{
			DBG2(DBG_TLS, "received %N acknowledgement packet",
				 eap_type_names, this->type);
			status = build_pkt(this, out);
			if (status == INVALID_STATE && this->tls->is_complete(this->tls))
			{
				return SUCCESS;
			}
			return status;
		}
		status = process_pkt(this, pkt);
		switch (status)
		{
			case NEED_MORE:
				break;
			case SUCCESS:
				return this->tls->is_complete(this->tls) ? SUCCESS : FAILED;
			default:
				return status;
		}
	}
	status = build_pkt(this, out);
	switch (status)
	{
		case INVALID_STATE:
			*out = create_ack(this);
			return NEED_MORE;
		case FAILED:
			if (!this->is_server)
			{
				*out = create_ack(this);
				return NEED_MORE;
			}
			return FAILED;
		default:
			return status;
	}
}

METHOD(tls_eap_t, get_msk, chunk_t,
	private_tls_eap_t *this)
{
	return this->tls->get_eap_msk(this->tls);
}

METHOD(tls_eap_t, get_identifier, uint8_t,
	private_tls_eap_t *this)
{
	return this->identifier;
}

METHOD(tls_eap_t, set_identifier, void,
	private_tls_eap_t *this, uint8_t identifier)
{
	this->identifier = identifier;
}

METHOD(tls_eap_t, destroy, void,
	private_tls_eap_t *this)
{
	this->tls->destroy(this->tls);
	free(this);
}

/**
 * See header
 */
tls_eap_t *tls_eap_create(eap_type_t type, tls_t *tls, size_t frag_size,
						  int max_msg_count, bool include_length)
{
	private_tls_eap_t *this;

	if (!tls)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.initiate = _initiate,
			.process = _process,
			.get_msk = _get_msk,
			.get_identifier = _get_identifier,
			.set_identifier = _set_identifier,
			.destroy = _destroy,
		},
		.type = type,
		.is_server = tls->is_server(tls),
		.first_fragment = (type != EAP_TNC && type != EAP_PT_EAP),
		.frag_size = frag_size,
		.max_msg_count = max_msg_count,
		.include_length = include_length,
		.tls = tls,
	);

	switch (type)
	{
		case EAP_TTLS:
			this->supported_version = EAP_TTLS_SUPPORTED_VERSION;
			break;
		case EAP_TNC:
			this->supported_version = EAP_TNC_SUPPORTED_VERSION;
			break;
		case EAP_PEAP:
			this->supported_version = EAP_PEAP_SUPPORTED_VERSION;
			break;
		case EAP_PT_EAP:
			this->supported_version = EAP_PT_EAP_SUPPORTED_VERSION;
			break;
		default:
			break;
	}

	if (this->is_server)
	{
		do
		{	/* start with non-zero random identifier */
			this->identifier = random();
		}
		while (!this->identifier);
	}

	return &this->public;
}

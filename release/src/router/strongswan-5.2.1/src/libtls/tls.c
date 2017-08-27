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

#include "tls.h"

#include <utils/debug.h>

#include "tls_protection.h"
#include "tls_compression.h"
#include "tls_fragmentation.h"
#include "tls_crypto.h"
#include "tls_server.h"
#include "tls_peer.h"

ENUM_BEGIN(tls_version_names, SSL_2_0, SSL_2_0,
	"SSLv2");
ENUM_NEXT(tls_version_names, SSL_3_0, TLS_1_2, SSL_2_0,
	"SSLv3",
	"TLS 1.0",
	"TLS 1.1",
	"TLS 1.2");
ENUM_END(tls_version_names, TLS_1_2);

ENUM(tls_content_type_names, TLS_CHANGE_CIPHER_SPEC, TLS_APPLICATION_DATA,
	"ChangeCipherSpec",
	"Alert",
	"Handshake",
	"ApplicationData",
);

ENUM_BEGIN(tls_handshake_type_names, TLS_HELLO_REQUEST, TLS_SERVER_HELLO,
	"HelloRequest",
	"ClientHello",
	"ServerHello");
ENUM_NEXT(tls_handshake_type_names,
		TLS_CERTIFICATE, TLS_CLIENT_KEY_EXCHANGE, TLS_SERVER_HELLO,
	"Certificate",
	"ServerKeyExchange",
	"CertificateRequest",
	"ServerHelloDone",
	"CertificateVerify",
	"ClientKeyExchange");
ENUM_NEXT(tls_handshake_type_names,
		TLS_FINISHED, TLS_FINISHED, TLS_CLIENT_KEY_EXCHANGE,
	"Finished");
ENUM_END(tls_handshake_type_names, TLS_FINISHED);

ENUM_BEGIN(tls_extension_names, TLS_EXT_SERVER_NAME, TLS_EXT_STATUS_REQUEST,
	"server name",
	"max fragment length",
	"client certificate url",
	"trusted ca keys",
	"truncated hmac",
	"status request");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_ELLIPTIC_CURVES, TLS_EXT_EC_POINT_FORMATS,
		TLS_EXT_STATUS_REQUEST,
	"elliptic curves",
	"ec point formats");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_SIGNATURE_ALGORITHMS, TLS_EXT_SIGNATURE_ALGORITHMS,
		TLS_EXT_EC_POINT_FORMATS,
	"signature algorithms");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_RENEGOTIATION_INFO, TLS_EXT_RENEGOTIATION_INFO,
		TLS_EXT_SIGNATURE_ALGORITHMS,
	"renegotiation info");
ENUM_END(tls_extension_names, TLS_EXT_RENEGOTIATION_INFO);

/**
 * TLS record
 */
typedef struct __attribute__((packed)) {
	u_int8_t type;
	u_int16_t version;
	u_int16_t length;
	char data[];
} tls_record_t;

typedef struct private_tls_t private_tls_t;

/**
 * Private data of an tls_protection_t object.
 */
struct private_tls_t {

	/**
	 * Public tls_t interface.
	 */
	tls_t public;

	/**
	 * Role this TLS stack acts as.
	 */
	bool is_server;

	/**
	 * Negotiated TLS version
	 */
	tls_version_t version;

	/**
	 * TLS stack purpose, as given to constructor
	 */
	tls_purpose_t purpose;

	/**
	 * TLS record protection layer
	 */
	tls_protection_t *protection;

	/**
	 * TLS record compression layer
	 */
	tls_compression_t *compression;

	/**
	 * TLS record fragmentation layer
	 */
	tls_fragmentation_t *fragmentation;

	/**
	 * TLS alert handler
	 */
	tls_alert_t *alert;

	/**
	 * TLS crypto helper context
	 */
	tls_crypto_t *crypto;

	/**
	 * TLS handshake protocol handler
	 */
	tls_handshake_t *handshake;

	/**
	 * TLS application data handler
	 */
	tls_application_t *application;

	/**
	 * Allocated input buffer
	 */
	chunk_t input;

	/**
	 * Number of bytes read in input buffer
	 */
	size_t inpos;

	/**
	 * Allocated output buffer
	 */
	chunk_t output;

	/**
	 * Number of bytes processed from output buffer
	 */
	size_t outpos;

	/**
	 * Position in partially received record header
	 */
	size_t headpos;

	/**
	 * Partial TLS record header received
	 */
	tls_record_t head;
};

/**
 * Described in header.
 */
void libtls_init(void)
{
	/* empty */
}

METHOD(tls_t, process, status_t,
	private_tls_t *this, void *buf, size_t buflen)
{
	tls_record_t *record;
	status_t status;
	u_int len;

	if (this->headpos)
	{	/* have a partial TLS record header, try to complete it */
		len = min(buflen, sizeof(this->head) - this->headpos);
		memcpy(((char*)&this->head) + this->headpos, buf, len);
		this->headpos += len;
		buflen -= len;
		buf += len;
		if (this->headpos == sizeof(this->head))
		{	/* header complete, allocate space with new header */
			len = untoh16(&this->head.length);
			this->input = chunk_alloc(len + sizeof(tls_record_t));
			memcpy(this->input.ptr, &this->head, sizeof(this->head));
			this->inpos = sizeof(this->head);
			this->headpos = 0;
		}
	}

	while (buflen)
	{
		if (this->input.len == 0)
		{
			while (buflen >= sizeof(tls_record_t))
			{
				/* try to process records inline */
				record = buf;
				len = untoh16(&record->length);

				if (len + sizeof(tls_record_t) > buflen)
				{	/* not a full record, read to buffer */
					this->input = chunk_alloc(len + sizeof(tls_record_t));
					this->inpos = 0;
					break;
				}
				DBG2(DBG_TLS, "processing TLS %N record (%d bytes)",
					 tls_content_type_names, record->type, len);
				status = this->protection->process(this->protection,
								record->type, chunk_create(record->data, len));
				if (status != NEED_MORE)
				{
					return status;
				}
				buf += len + sizeof(tls_record_t);
				buflen -= len + sizeof(tls_record_t);
				if (buflen == 0)
				{
					return NEED_MORE;
				}
			}
			if (buflen < sizeof(tls_record_t))
			{
				DBG2(DBG_TLS, "received incomplete TLS record header");
				memcpy(&this->head, buf, buflen);
				this->headpos = buflen;
				break;
			}
		}
		len = min(buflen, this->input.len - this->inpos);
		memcpy(this->input.ptr + this->inpos, buf, len);
		buf += len;
		buflen -= len;
		this->inpos += len;
		DBG2(DBG_TLS, "buffering %d bytes, %d bytes of %d byte TLS record received",
			 len, this->inpos, this->input.len);
		if (this->input.len == this->inpos)
		{
			record = (tls_record_t*)this->input.ptr;
			len = untoh16(&record->length);

			DBG2(DBG_TLS, "processing buffered TLS %N record (%d bytes)",
				 tls_content_type_names, record->type, len);
			status = this->protection->process(this->protection,
								record->type, chunk_create(record->data, len));
			chunk_free(&this->input);
			this->inpos = 0;
			if (status != NEED_MORE)
			{
				return status;
			}
		}
	}
	return NEED_MORE;
}

METHOD(tls_t, build, status_t,
	private_tls_t *this, void *buf, size_t *buflen, size_t *msglen)
{
	tls_content_type_t type;
	tls_record_t record;
	status_t status;
	chunk_t data;
	size_t len;

	len = *buflen;
	if (this->output.len == 0)
	{
		/* query upper layers for new records, as many as we can get */
		while (TRUE)
		{
			status = this->protection->build(this->protection, &type, &data);
			switch (status)
			{
				case NEED_MORE:
					record.type = type;
					htoun16(&record.version, this->version);
					htoun16(&record.length, data.len);
					this->output = chunk_cat("mcm", this->output,
											 chunk_from_thing(record), data);
					DBG2(DBG_TLS, "sending TLS %N record (%d bytes)",
						 tls_content_type_names, type, data.len);
					continue;
				case INVALID_STATE:
					if (this->output.len == 0)
					{
						return INVALID_STATE;
					}
					break;
				default:
					return status;
			}
			break;
		}
		if (msglen)
		{
			*msglen = this->output.len;
		}
	}
	else
	{
		if (msglen)
		{
			*msglen = 0;
		}
	}
	len = min(len, this->output.len - this->outpos);
	memcpy(buf, this->output.ptr + this->outpos, len);
	this->outpos += len;
	*buflen = len;
	if (this->outpos == this->output.len)
	{
		chunk_free(&this->output);
		this->outpos = 0;
		return ALREADY_DONE;
	}
	return NEED_MORE;
}

METHOD(tls_t, is_server, bool,
	private_tls_t *this)
{
	return this->is_server;
}

METHOD(tls_t, get_server_id, identification_t*,
	private_tls_t *this)
{
	return this->handshake->get_server_id(this->handshake);
}

METHOD(tls_t, get_peer_id, identification_t*,
	private_tls_t *this)
{
	return this->handshake->get_peer_id(this->handshake);
}

METHOD(tls_t, get_version, tls_version_t,
	private_tls_t *this)
{
	return this->version;
}

METHOD(tls_t, set_version, bool,
	private_tls_t *this, tls_version_t version)
{
	if (version > this->version)
	{
		return FALSE;
	}
	switch (version)
	{
		case TLS_1_0:
		case TLS_1_1:
		case TLS_1_2:
			this->version = version;
			this->protection->set_version(this->protection, version);
			return TRUE;
		case SSL_2_0:
		case SSL_3_0:
		default:
			return FALSE;
	}
}

METHOD(tls_t, get_purpose, tls_purpose_t,
	private_tls_t *this)
{
	return this->purpose;
}

METHOD(tls_t, is_complete, bool,
	private_tls_t *this)
{
	if (this->handshake->finished(this->handshake))
	{
		if (!this->application)
		{
			return TRUE;
		}
		return this->fragmentation->application_finished(this->fragmentation);
	}
	return FALSE;
}

METHOD(tls_t, get_eap_msk, chunk_t,
	private_tls_t *this)
{
	return this->crypto->get_eap_msk(this->crypto);
}

METHOD(tls_t, destroy, void,
	private_tls_t *this)
{
	this->protection->destroy(this->protection);
	this->compression->destroy(this->compression);
	this->fragmentation->destroy(this->fragmentation);
	this->crypto->destroy(this->crypto);
	this->handshake->destroy(this->handshake);
	DESTROY_IF(this->application);
	this->alert->destroy(this->alert);

	free(this->input.ptr);
	free(this->output.ptr);

	free(this);
}

/**
 * See header
 */
tls_t *tls_create(bool is_server, identification_t *server,
				  identification_t *peer, tls_purpose_t purpose,
				  tls_application_t *application, tls_cache_t *cache)
{
	private_tls_t *this;

	switch (purpose)
	{
		case TLS_PURPOSE_EAP_TLS:
		case TLS_PURPOSE_EAP_TTLS:
		case TLS_PURPOSE_EAP_PEAP:
		case TLS_PURPOSE_GENERIC:
		case TLS_PURPOSE_GENERIC_NULLOK:
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.process = _process,
			.build = _build,
			.is_server = _is_server,
			.get_server_id = _get_server_id,
			.get_peer_id = _get_peer_id,
			.get_version = _get_version,
			.set_version = _set_version,
			.get_purpose = _get_purpose,
			.is_complete = _is_complete,
			.get_eap_msk = _get_eap_msk,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.version = TLS_1_2,
		.application = application,
		.purpose = purpose,
	);
	lib->settings->add_fallback(lib->settings, "%s.tls", "libtls", lib->ns);

	this->crypto = tls_crypto_create(&this->public, cache);
	this->alert = tls_alert_create();
	if (is_server)
	{
		this->handshake = &tls_server_create(&this->public, this->crypto,
										this->alert, server, peer)->handshake;
	}
	else
	{
		this->handshake = &tls_peer_create(&this->public, this->crypto,
										this->alert, peer, server)->handshake;
	}
	this->fragmentation = tls_fragmentation_create(this->handshake, this->alert,
												   this->application);
	this->compression = tls_compression_create(this->fragmentation, this->alert);
	this->protection = tls_protection_create(this->compression, this->alert);
	this->crypto->set_protection(this->crypto, this->protection);

	return &this->public;
}

/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2020-2021 Pascal Knecht
 * Copyright (C) 2010 Martin Willi
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

#include "tls.h"

#include <utils/debug.h>

#include "tls_protection.h"
#include "tls_compression.h"
#include "tls_fragmentation.h"
#include "tls_crypto.h"
#include "tls_server.h"
#include "tls_peer.h"

ENUM_BEGIN(tls_version_names, TLS_UNSPEC, TLS_UNSPEC,
	"TLS UNSPEC");
ENUM_NEXT(tls_version_names, SSL_2_0, SSL_2_0, TLS_UNSPEC,
	"SSLv2");
ENUM_NEXT(tls_version_names, SSL_3_0, TLS_1_3, SSL_2_0,
	"SSLv3",
	"TLS 1.0",
	"TLS 1.1",
	"TLS 1.2",
	"TLS 1.3");
ENUM_END(tls_version_names, TLS_1_3);

/**
 * Only supported versions are mapped
 */
ENUM(tls_numeric_version_names, TLS_SUPPORTED_MIN, TLS_SUPPORTED_MAX,
	"1.0",
	"1.1",
	"1.2",
	"1.3");

ENUM(tls_content_type_names, TLS_CHANGE_CIPHER_SPEC, TLS_APPLICATION_DATA,
	"ChangeCipherSpec",
	"Alert",
	"Handshake",
	"ApplicationData",
);

ENUM_BEGIN(tls_handshake_type_names, TLS_HELLO_REQUEST, TLS_HELLO_REQUEST,
	"HelloRequest");
ENUM_NEXT(tls_handshake_type_names,
		TLS_CLIENT_HELLO, TLS_HELLO_RETRY_REQUEST, TLS_HELLO_REQUEST,
	"ClientHello",
	"ServerHello",
	"HelloVerifyRequest",
	"NewSessionTicket",
	"EndOfEarlyData",
	"HelloRetryRequest");
ENUM_NEXT(tls_handshake_type_names,
		TLS_ENCRYPTED_EXTENSIONS, TLS_ENCRYPTED_EXTENSIONS,
		TLS_HELLO_RETRY_REQUEST,
	"EncryptedExtensions");
ENUM_NEXT(tls_handshake_type_names,
		TLS_CERTIFICATE, TLS_CLIENT_KEY_EXCHANGE, TLS_ENCRYPTED_EXTENSIONS,
	"Certificate",
	"ServerKeyExchange",
	"CertificateRequest",
	"ServerHelloDone",
	"CertificateVerify",
	"ClientKeyExchange");
ENUM_NEXT(tls_handshake_type_names,
		  TLS_FINISHED, TLS_KEY_UPDATE, TLS_CLIENT_KEY_EXCHANGE,
	"Finished",
	"CertificateUrl",
	"CertificateStatus",
	"SupplementalData",
	"KeyUpdate");
ENUM_NEXT(tls_handshake_type_names,
		TLS_MESSAGE_HASH, TLS_MESSAGE_HASH, TLS_KEY_UPDATE,
	"MessageHash");
ENUM_END(tls_handshake_type_names, TLS_MESSAGE_HASH);

ENUM_BEGIN(tls_extension_names, TLS_EXT_SERVER_NAME, TLS_EXT_STATUS_REQUEST,
	"server name",
	"max fragment length",
	"client certificate url",
	"trusted ca keys",
	"truncated hmac",
	"status request");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_SUPPORTED_GROUPS, TLS_EXT_EC_POINT_FORMATS,
		TLS_EXT_STATUS_REQUEST,
	"supported groups",
	"ec point formats");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_SIGNATURE_ALGORITHMS,
		TLS_EXT_APPLICATION_LAYER_PROTOCOL_NEGOTIATION,
		TLS_EXT_EC_POINT_FORMATS,
	"signature algorithms",
	"use rtp",
	"heartbeat",
	"application layer protocol negotiation");
ENUM_NEXT(tls_extension_names,
		TLS_CLIENT_CERTIFICATE_TYPE, TLS_SERVER_CERTIFICATE_TYPE,
		TLS_EXT_APPLICATION_LAYER_PROTOCOL_NEGOTIATION,
	"client certificate type",
	"server certificate type");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_ENCRYPT_THEN_MAC, TLS_EXT_EXTENDED_MASTER_SECRET,
		TLS_SERVER_CERTIFICATE_TYPE,
	"encrypt-then-mac",
	"extended master secret");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_SESSION_TICKET, TLS_EXT_SESSION_TICKET,
		TLS_EXT_EXTENDED_MASTER_SECRET,
	"session ticket");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_PRE_SHARED_KEY, TLS_EXT_PSK_KEY_EXCHANGE_MODES,
		TLS_EXT_SESSION_TICKET,
	"pre-shared key",
	"early data",
	"supported versions",
	"cookie",
	"psk key exchange modes");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_CERTIFICATE_AUTHORITIES, TLS_EXT_KEY_SHARE,
		TLS_EXT_PSK_KEY_EXCHANGE_MODES,
	"certificate authorities",
	"oid filters",
	"post-handshake auth",
	"signature algorithms cert",
	"key-share");
ENUM_NEXT(tls_extension_names,
		TLS_EXT_RENEGOTIATION_INFO, TLS_EXT_RENEGOTIATION_INFO,
		TLS_EXT_KEY_SHARE,
	"renegotiation info");
ENUM_END(tls_extension_names, TLS_EXT_RENEGOTIATION_INFO);

chunk_t tls_hello_retry_request_magic = chunk_from_chars(
	0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11,
	0xBE, 0x1D, 0x8C, 0x02, 0x1E, 0x65, 0xB8, 0x91,
	0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E,
	0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C,
);

chunk_t tls_downgrade_protection_tls11 = chunk_from_chars(
	0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x00,
);
chunk_t tls_downgrade_protection_tls12 = chunk_from_chars(
	0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x01,
);

/**
 * TLS record
 */
typedef struct __attribute__((packed)) {
	uint8_t type;
	uint16_t version;
	uint16_t length;
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
	 * Negotiated TLS version and maximum supported TLS version
	 */
	tls_version_t version_max;

	/**
	 * Minimal supported TLS version
	 */
	tls_version_t version_min;

	/**
	 * TLS stack purpose, as given to constructor
	 */
	tls_purpose_t purpose;

	/**
	 * Flags for this TLS stack
	 */
	tls_flag_t flags;

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
					if (this->version_max < TLS_1_3)
					{
						htoun16(&record.version, this->version_max);
					}
					else
					{
						htoun16(&record.version, TLS_1_2);
					}
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

/**
 * Determine the min/max versions
 */
static void determine_versions(private_tls_t *this)
{
	tls_version_t version;
	char *version_str;

	if (this->version_min == TLS_UNSPEC)
	{	/* default to TLS 1.2 as older versions are considered deprecated */
		this->version_min = TLS_1_2;

		version_str = lib->settings->get_str(lib->settings, "%s.tls.version_min",
											 NULL, lib->ns);
		if (version_str &&
			enum_from_name(tls_numeric_version_names, version_str, &version))
		{
			this->version_min = version;
		}
	}
	if (this->version_max == TLS_UNSPEC)
	{	/* default to TLS 1.2 until 1.3 is stable for use in EAP */
		this->version_max = TLS_1_2;

		version_str = lib->settings->get_str(lib->settings, "%s.tls.version_max",
											 NULL, lib->ns);
		if (version_str &&
			enum_from_name(tls_numeric_version_names, version_str, &version))
		{
			this->version_max = version;
		}
	}
	if (this->version_max < this->version_min)
	{
		this->version_min = this->version_max;
	}
}

METHOD(tls_t, get_version_max, tls_version_t,
	private_tls_t *this)
{
	determine_versions(this);
	return this->version_max;
}

METHOD(tls_t, get_version_min, tls_version_t,
	private_tls_t *this)
{
	determine_versions(this);
	return this->version_min;
}

METHOD(tls_t, set_version, bool,
	private_tls_t *this, tls_version_t min_version, tls_version_t max_version)
{
	if (min_version == TLS_UNSPEC)
	{
		min_version = this->version_min;
	}
	if (max_version == TLS_UNSPEC)
	{
		max_version = this->version_max;
	}
	if ((this->version_min != TLS_UNSPEC && min_version < this->version_min) ||
		(this->version_max != TLS_UNSPEC && max_version > this->version_max) ||
		(min_version != TLS_UNSPEC && min_version < TLS_SUPPORTED_MIN) ||
		(max_version != TLS_UNSPEC && max_version > TLS_SUPPORTED_MAX) ||
		min_version > max_version)
	{
		return FALSE;
	}

	this->version_min = min_version;
	this->version_max = max_version;

	if (min_version != TLS_UNSPEC && min_version == max_version)
	{
		this->protection->set_version(this->protection, max_version);
	}
	return TRUE;
}

METHOD(tls_t, get_purpose, tls_purpose_t,
	private_tls_t *this)
{
	return this->purpose;
}

METHOD(tls_t, get_flags, tls_flag_t,
	private_tls_t *this)
{
	return this->flags;
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

METHOD(tls_t, get_auth, auth_cfg_t*,
	private_tls_t *this)
{
	return this->handshake->get_auth(this->handshake);
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
				  tls_application_t *application, tls_cache_t *cache,
				  tls_flag_t flags)
{
	private_tls_t *this;

	switch (purpose)
	{
		case TLS_PURPOSE_EAP_TLS:
		case TLS_PURPOSE_EAP_TTLS:
		case TLS_PURPOSE_EAP_PEAP:
		case TLS_PURPOSE_GENERIC:
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
			.get_version_max = _get_version_max,
			.get_version_min = _get_version_min,
			.set_version = _set_version,
			.get_purpose = _get_purpose,
			.get_flags = _get_flags,
			.is_complete = _is_complete,
			.get_eap_msk = _get_eap_msk,
			.get_auth = _get_auth,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.application = application,
		.purpose = purpose,
		.flags = flags,
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
												   this->application, purpose);
	this->compression = tls_compression_create(this->fragmentation, this->alert);
	this->protection = tls_protection_create(this->compression, this->alert);
	this->crypto->set_protection(this->crypto, this->protection);

	return &this->public;
}

/*
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2020-2021 Pascal Knecht
 * Copyright (C) 2020 Méline Sieber
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

#include "tls_peer.h"

#include <utils/debug.h>

#include <time.h>

typedef struct private_tls_peer_t private_tls_peer_t;

typedef enum {
	STATE_INIT,
	STATE_HELLO_SENT,
	STATE_HELLO_RECEIVED,
	STATE_HELLO_DONE,
	STATE_CERT_SENT,
	STATE_CERT_RECEIVED,
	STATE_KEY_EXCHANGE_RECEIVED,
	STATE_CERTREQ_RECEIVED,
	STATE_KEY_EXCHANGE_SENT,
	STATE_VERIFY_SENT,
	STATE_CIPHERSPEC_CHANGED_OUT,
	STATE_FINISHED_SENT,
	STATE_CIPHERSPEC_CHANGED_IN,
	STATE_FINISHED_RECEIVED,
	/* new states in TLS 1.3 */
	STATE_HELLORETRYREQ_RECEIVED,
	STATE_ENCRYPTED_EXTENSIONS_RECEIVED,
	STATE_CERT_VERIFY_RECEIVED,
	STATE_FINISHED_SENT_KEY_SWITCHED,
	STATE_KEY_UPDATE_REQUESTED,
	STATE_KEY_UPDATE_SENT,
	STATE_CERT_VERIFY_SENT,
} peer_state_t;

/**
 * Private data of an tls_peer_t object.
 */
struct private_tls_peer_t {

	/**
	 * Public tls_peer_t interface.
	 */
	tls_peer_t public;

	/**
	 * TLS stack
	 */
	tls_t *tls;

	/**
	 * TLS crypto context
	 */
	tls_crypto_t *crypto;

	/**
	 * TLS alert handler
	 */
	tls_alert_t *alert;

	/**
	 * Peer identity, NULL for no client authentication
	 */
	identification_t *peer;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * State we are in
	 */
	peer_state_t state;

	/**
	 *  Received a certificate request from server
	 */
	 bool certreq_received;

	/**
	 * TLS version we offered in hello
	 */
	tls_version_t hello_version;

	/**
	 * Hello random data selected by client
	 */
	char client_random[32];

	/**
	 * Hello random data selected by server
	 */
	char server_random[32];

	/**
	 * Auth helper for peer authentication
	 */
	auth_cfg_t *peer_auth;

	/**
	 * Auth helper for server authentication
	 */
	auth_cfg_t *server_auth;

	/**
	 * Peer private key
	 */
	private_key_t *private;

	/**
	 * DHE exchange
	 */
	key_exchange_t *dh;

	/**
	 * Requested DH group
	 */
	tls_named_group_t requested_curve;

	/**
	 * Original cipher suite in HelloRetryRequest
	 */
	tls_cipher_suite_t original_suite;

	/**
	 * Cookie extension received in HelloRetryRequest
	 */
	chunk_t cookie;

	/**
	 * Resuming a session?
	 */
	bool resume;

	/**
	 * TLS session identifier
	 */
	chunk_t session;

	/**
	 * List of server-supported hashsig algorithms
	 */
	chunk_t hashsig;

	/**
	 * List of server-supported client certificate types
	 */
	chunk_t cert_types;
};

/* Implemented in tls_server.c */
bool tls_write_key_share(bio_writer_t **key_share, key_exchange_t *dh);
public_key_t *tls_find_public_key(auth_cfg_t *peer_auth, identification_t *id);

/**
 * Verify the DH group/key type requested by the server is valid.
 */
static bool verify_requested_key_type(private_tls_peer_t *this,
									  uint16_t key_type)
{
	enumerator_t *enumerator;
	key_exchange_method_t group, found = KE_NONE;
	tls_named_group_t curve;

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, &group, &curve))
	{
		if (key_type == curve)
		{
			found = group;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found == KE_NONE)
	{
		DBG1(DBG_TLS, "server requested key exchange we didn't propose");
		return FALSE;
	}
	if (this->dh->get_method(this->dh) == found)
	{
		DBG1(DBG_TLS, "server requested key exchange we already use");
		return FALSE;
	}
	return TRUE;
}

/**
 * Process a server hello message
 */
static status_t process_server_hello(private_tls_peer_t *this,
									 bio_reader_t *reader)
{
	uint8_t compression;
	uint16_t version, cipher, key_type = 0;
	bio_reader_t *extensions, *extension;
	chunk_t msg, random, session, ext = chunk_empty, key_share = chunk_empty;
	chunk_t cookie = chunk_empty;
	tls_cipher_suite_t suite = 0;
	tls_version_t version_max;
	bool is_retry_request;

	msg = reader->peek(reader);
	if (!reader->read_uint16(reader, &version) ||
		!reader->read_data(reader, sizeof(this->server_random), &random) ||
		!reader->read_data8(reader, &session) ||
		!reader->read_uint16(reader, &cipher) ||
		!reader->read_uint8(reader, &compression) ||
		(reader->remaining(reader) && !reader->read_data16(reader, &ext)))
	{
		DBG1(DBG_TLS, "received invalid ServerHello");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	is_retry_request = chunk_equals_const(random, tls_hello_retry_request_magic);

	memcpy(this->server_random, random.ptr, sizeof(this->server_random));

	extensions = bio_reader_create(ext);
	while (extensions->remaining(extensions))
	{
		uint16_t extension_type;
		chunk_t extension_data;

		if (!extensions->read_uint16(extensions, &extension_type) ||
			!extensions->read_data16(extensions, &extension_data))
		{
			DBG1(DBG_TLS, "invalid extension in ServerHello");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			extensions->destroy(extensions);
			return NEED_MORE;
		}
		extension = bio_reader_create(extension_data);
		switch (extension_type)
		{
			case TLS_EXT_SUPPORTED_VERSIONS:
				if (!extension->read_uint16(extension, &version))
				{
					DBG1(DBG_TLS, "invalid %N extension", tls_extension_names,
						 extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				break;
			case TLS_EXT_KEY_SHARE:
				if (!extension->read_uint16(extension, &key_type) ||
					(!is_retry_request &&
					 !(extension->read_data16(extension, &key_share) &&
					   key_share.len)))
				{
					DBG1(DBG_TLS, "invalid %N extension", tls_extension_names,
						 extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				break;
			case TLS_EXT_COOKIE:
				if (!extension->read_data16(extension, &cookie))
				{
					DBG1(DBG_TLS, "invalid %N extension", tls_extension_names,
						 extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
			default:
				break;
		}
		extension->destroy(extension);
	}
	extensions->destroy(extensions);

	/* downgrade protection (see RFC 8446, section 4.1.3) */
	version_max = this->tls->get_version_max(this->tls);
	if ((version_max == TLS_1_3 && version < TLS_1_3) ||
		(version_max == TLS_1_2 && version < TLS_1_2))
	{
		chunk_t server_random_end = chunk_create(&this->server_random[24], 8);

		if (chunk_equals(server_random_end, tls_downgrade_protection_tls11) ||
			chunk_equals(server_random_end, tls_downgrade_protection_tls12))
		{
			DBG1(DBG_TLS, "server random indicates downgrade attack to %N",
				 tls_version_names, version);
			this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
			return NEED_MORE;
		}
	}

	if (!this->tls->set_version(this->tls, version, version))
	{
		DBG1(DBG_TLS, "negotiated version %N not supported",
			 tls_version_names, version);
		this->alert->add(this->alert, TLS_FATAL, TLS_PROTOCOL_VERSION);
		return NEED_MORE;
	}

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (chunk_equals(this->session, session))
		{
			suite = this->crypto->resume_session(this->crypto, session,
												 this->server, chunk_from_thing
												 (this->client_random),
												 chunk_from_thing
												 (this->server_random));
			if (suite)
			{
				DBG1(DBG_TLS, "resumed %N using suite %N",
					 tls_version_names, version, tls_cipher_suite_names, suite);
				this->resume = TRUE;
			}
		}
		DESTROY_IF(this->dh);
		this->dh = NULL;
	}

	if (!suite)
	{
		suite = cipher;
		if (!this->crypto->select_cipher_suite(this->crypto, &suite, 1, KEY_ANY))
		{
			DBG1(DBG_TLS, "received TLS cipher suite %N unacceptable",
				 tls_cipher_suite_names, suite);
			this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
			return NEED_MORE;
		}
		if (this->original_suite && this->original_suite != suite)
		{
			DBG1(DBG_TLS, "server selected %N instead of %N after retry",
				 tls_cipher_suite_names, suite, tls_cipher_suite_names,
				 this->original_suite);
			this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
			return NEED_MORE;
		}
		DBG1(DBG_TLS, "negotiated %N using suite %N",
			 tls_version_names, version, tls_cipher_suite_names, suite);
		free(this->session.ptr);
		this->session = chunk_clone(session);
	}

	if (is_retry_request)
	{
		if (!this->crypto->hash_handshake(this->crypto, NULL))
		{
			DBG1(DBG_TLS, "failed to hash handshake messages");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
	}
	this->crypto->append_handshake(this->crypto, TLS_SERVER_HELLO, msg);

	if (is_retry_request)
	{
		if (key_type)
		{
			DBG1(DBG_TLS, "server requests key exchange with %N",
				 tls_named_group_names, key_type);
		}
		else if (cookie.len)
		{
			DBG1(DBG_TLS, "server requests retry with cookie");
		}
		else
		{
			DBG1(DBG_TLS, "invalid retry request received");
			this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
			return NEED_MORE;
		}
		if (this->requested_curve || this->cookie.len)
		{
			DBG1(DBG_TLS, "already replied to previous retry request");
			this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
			return NEED_MORE;
		}
		if (key_type && !verify_requested_key_type(this, key_type))
		{
			this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
			return NEED_MORE;
		}

		DESTROY_IF(this->dh);
		this->dh = NULL;
		this->original_suite = suite;
		this->requested_curve = key_type;
		this->cookie = chunk_clone(cookie);
		this->state = STATE_INIT;
		return NEED_MORE;
	}

	if (this->tls->get_version_max(this->tls) >= TLS_1_3)
	{
		chunk_t shared_secret = chunk_empty;

		if (key_share.len &&
			key_type != TLS_CURVE25519 &&
			key_type != TLS_CURVE448)
		{	/* classic format (see RFC 8446, section 4.2.8.2) */
			if (key_share.ptr[0] != TLS_ANSI_UNCOMPRESSED)
			{
				DBG1(DBG_TLS, "DH point format '%N' not supported",
					 tls_ansi_point_format_names, key_share.ptr[0]);
				this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
				return NEED_MORE;
			}
			key_share = chunk_skip(key_share, 1);
		}
		if (!key_share.len ||
			!this->dh->set_public_key(this->dh, key_share) ||
			!this->dh->get_shared_secret(this->dh, &shared_secret) ||
			!this->crypto->derive_handshake_keys(this->crypto, shared_secret))
		{
			DBG1(DBG_TLS, "DH key derivation failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
			chunk_clear(&shared_secret);
			return NEED_MORE;
		}
		chunk_clear(&shared_secret);

		this->crypto->change_cipher(this->crypto, TRUE);
		this->crypto->change_cipher(this->crypto, FALSE);
	}

	this->state = STATE_HELLO_RECEIVED;
	return NEED_MORE;
}

/**
 * Process a server encrypted extensions message
 */
static status_t process_encrypted_extensions(private_tls_peer_t *this,
											 bio_reader_t *reader)
{
	chunk_t ext = chunk_empty;
	uint16_t extension_type;

	this->crypto->append_handshake(this->crypto, TLS_ENCRYPTED_EXTENSIONS,
								   reader->peek(reader));

	if (!reader->read_data16(reader, &ext))
	{
		DBG1(DBG_TLS, "received invalid EncryptedExtensions");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	if (ext.len)
	{
		bio_reader_t *extensions = bio_reader_create(ext);

		while (extensions->remaining(extensions))
		{
			chunk_t extension_data = chunk_empty;

			if (!extensions->read_uint16(extensions, &extension_type) ||
				!extensions->read_data16(extensions, &extension_data))
			{
				DBG1(DBG_TLS, "invalid extension in EncryptedExtensions");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				extensions->destroy(extensions);
				return NEED_MORE;
			}
			switch (extension_type)
			{
				case TLS_EXT_SERVER_NAME:
				case TLS_EXT_MAX_FRAGMENT_LENGTH:
				case TLS_EXT_SUPPORTED_GROUPS:
				case TLS_EXT_USE_SRTP:
				case TLS_EXT_HEARTBEAT:
				case TLS_EXT_APPLICATION_LAYER_PROTOCOL_NEGOTIATION:
				case TLS_SERVER_CERTIFICATE_TYPE:
					/* not supported so far */
					DBG2(DBG_TLS, "ignoring unsupported %N EncryptedExtension",
						 tls_extension_names, extension_type);
					break;
				default:
					DBG1(DBG_TLS, "received forbidden EncryptedExtension (%d)",
						 extension_type);
					this->alert->add(this->alert, TLS_FATAL,
									 TLS_ILLEGAL_PARAMETER);
					extensions->destroy(extensions);
					return NEED_MORE;
			}
		}
		extensions->destroy(extensions);
	}
	this->state = STATE_ENCRYPTED_EXTENSIONS_RECEIVED;
	return NEED_MORE;
}

/**
 * Process a Certificate message
 */
static status_t process_certificate(private_tls_peer_t *this,
									bio_reader_t *reader)
{
	certificate_t *cert;
	bio_reader_t *certs;
	chunk_t data;
	bool first = TRUE;

	this->crypto->append_handshake(this->crypto,
								   TLS_CERTIFICATE, reader->peek(reader));

	if (this->tls->get_version_max(this->tls) > TLS_1_2)
	{
		if (!reader->read_data8(reader, &data))
		{
			DBG1(DBG_TLS, "certificate request context invalid");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
	}

	if (!reader->read_data24(reader, &data))
	{
		DBG1(DBG_TLS, "certificate message header invalid");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	certs = bio_reader_create(data);
	while (certs->remaining(certs))
	{
		if (!certs->read_data24(certs, &data))
		{
			DBG1(DBG_TLS, "certificate message invalid");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			certs->destroy(certs);
			return NEED_MORE;
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB_ASN1_DER, data, BUILD_END);
		if (cert)
		{
			if (first)
			{
				if (!cert->has_subject(cert, this->server))
				{
					DBG1(DBG_TLS, "server certificate does not match to '%Y'",
						 this->server);
					cert->destroy(cert);
					certs->destroy(certs);
					this->alert->add(this->alert, TLS_FATAL, TLS_ACCESS_DENIED);
					return NEED_MORE;
				}
				this->server_auth->add(this->server_auth,
									   AUTH_HELPER_SUBJECT_CERT, cert);
				DBG1(DBG_TLS, "received TLS server certificate '%Y'",
					 cert->get_subject(cert));
				first = FALSE;
			}
			else
			{
				DBG1(DBG_TLS, "received TLS intermediate certificate '%Y'",
					 cert->get_subject(cert));
				this->server_auth->add(this->server_auth,
									   AUTH_HELPER_IM_CERT, cert);
			}
		}
		else
		{
			DBG1(DBG_TLS, "parsing TLS certificate failed, skipped");
			this->alert->add(this->alert, TLS_WARNING, TLS_BAD_CERTIFICATE);
		}
		if (this->tls->get_version_max(this->tls) > TLS_1_2)
		{
			if (!certs->read_data16(certs, &data))
			{
				DBG1(DBG_TLS, "failed to read extensions of CertificateEntry");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				return NEED_MORE;
			}
		}
	}
	certs->destroy(certs);
	this->state = STATE_CERT_RECEIVED;
	return NEED_MORE;
}

/**
 *  Process CertificateVerify message
 */
static status_t process_cert_verify(private_tls_peer_t *this,
									bio_reader_t *reader)
{
	public_key_t *public;
	chunk_t msg;

	public = tls_find_public_key(this->server_auth, this->server);
	if (!public)
	{
		DBG1(DBG_TLS, "no trusted certificate found for '%Y' to verify TLS server",
			 this->server);
		this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
		return NEED_MORE;
	}

	msg = reader->peek(reader);
	if (!this->crypto->verify_handshake(this->crypto, public, reader))
	{
		public->destroy(public);
		DBG1(DBG_TLS, "signature verification failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_BAD_CERTIFICATE);
		return NEED_MORE;
	}
	public->destroy(public);

	this->crypto->append_handshake(this->crypto, TLS_CERTIFICATE_VERIFY, msg);
	this->state = STATE_CERT_VERIFY_RECEIVED;
	return NEED_MORE;
}

/**
 * Process a Key Exchange message using MODP Diffie Hellman
 */
static status_t process_modp_key_exchange(private_tls_peer_t *this,
										  bio_reader_t *reader)
{
	chunk_t prime, generator, pub, chunk;
	public_key_t *public;

	chunk = reader->peek(reader);
	if (!reader->read_data16(reader, &prime) ||
		!reader->read_data16(reader, &generator) ||
		!reader->read_data16(reader, &pub))
	{
		DBG1(DBG_TLS, "received invalid Server Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	/* reject (export) DH groups using primes smaller than 1024 bit */
	if (prime.len < 1024 / 8)
	{
		DBG1(DBG_TLS, "short DH prime received (%zu bytes)", prime.len);
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	public = tls_find_public_key(this->server_auth, this->server);
	if (!public)
	{
		DBG1(DBG_TLS, "no TLS public key found for server '%Y'", this->server);
		this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
		return NEED_MORE;
	}

	chunk.len = 2 + prime.len + 2 + generator.len + 2 + pub.len;
	chunk = chunk_cat("ccc", chunk_from_thing(this->client_random),
					  chunk_from_thing(this->server_random), chunk);
	if (!this->crypto->verify(this->crypto, public, reader, chunk))
	{
		public->destroy(public);
		free(chunk.ptr);
		DBG1(DBG_TLS, "verifying DH parameters failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_BAD_CERTIFICATE);
		return NEED_MORE;
	}
	public->destroy(public);
	free(chunk.ptr);

	this->dh = lib->crypto->create_ke(lib->crypto, MODP_CUSTOM,
									  generator, prime);
	if (!this->dh)
	{
		DBG1(DBG_TLS, "custom DH parameters not supported");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (!this->dh->set_public_key(this->dh, pub))
	{
		DBG1(DBG_TLS, "applying DH public value failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	this->state = STATE_KEY_EXCHANGE_RECEIVED;
	return NEED_MORE;
}

/**
 * Get the EC group for a TLS named curve
 */
static key_exchange_method_t curve_to_ec_group(private_tls_peer_t *this,
											   tls_named_group_t curve)
{
	key_exchange_method_t group;
	tls_named_group_t current;
	enumerator_t *enumerator;

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, &group, &current))
	{
		if (current == curve)
		{
			enumerator->destroy(enumerator);
			return group;
		}
	}
	enumerator->destroy(enumerator);
	return 0;
}

/**
 * Process a Key Exchange message using EC Diffie Hellman
 */
static status_t process_ec_key_exchange(private_tls_peer_t *this,
										bio_reader_t *reader)
{
	key_exchange_method_t group;
	public_key_t *public;
	uint8_t type;
	uint16_t curve;
	chunk_t pub, chunk;

	chunk = reader->peek(reader);
	if (!reader->read_uint8(reader, &type))
	{
		DBG1(DBG_TLS, "received invalid Server Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	if (type != TLS_ECC_NAMED_CURVE)
	{
		DBG1(DBG_TLS, "ECDH curve type %N not supported",
			 tls_ecc_curve_type_names, type);
		this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
		return NEED_MORE;
	}
	if (!reader->read_uint16(reader, &curve) ||
		!reader->read_data8(reader, &pub) || pub.len == 0)
	{
		DBG1(DBG_TLS, "received invalid Server Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	group = curve_to_ec_group(this, curve);
	if (!group)
	{
		DBG1(DBG_TLS, "ECDH curve %N not supported",
			 tls_named_group_names, curve);
		this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
		return NEED_MORE;
	}

	public = tls_find_public_key(this->server_auth, this->server);
	if (!public)
	{
		DBG1(DBG_TLS, "no TLS public key found for server '%Y'", this->server);
		this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
		return NEED_MORE;
	}

	chunk.len = 4 + pub.len;
	chunk = chunk_cat("ccc", chunk_from_thing(this->client_random),
					  chunk_from_thing(this->server_random), chunk);
	if (!this->crypto->verify(this->crypto, public, reader, chunk))
	{
		public->destroy(public);
		free(chunk.ptr);
		DBG1(DBG_TLS, "verifying DH parameters failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_BAD_CERTIFICATE);
		return NEED_MORE;
	}
	public->destroy(public);
	free(chunk.ptr);

	this->dh = lib->crypto->create_ke(lib->crypto, group);
	if (!this->dh)
	{
		DBG1(DBG_TLS, "DH group %N not supported",
			 key_exchange_method_names, group);
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	if (group != CURVE_25519 &&
		group != CURVE_448)
	{	/* classic ECPoint format (see RFC 8422, section 5.4.1) */
		if (pub.ptr[0] != TLS_ANSI_UNCOMPRESSED)
		{
			DBG1(DBG_TLS, "DH point format '%N' not supported",
				 tls_ansi_point_format_names, pub.ptr[0]);
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		pub = chunk_skip(pub, 1);
	}

	if (!this->dh->set_public_key(this->dh, pub))
	{
		DBG1(DBG_TLS, "applying DH public value failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	this->state = STATE_KEY_EXCHANGE_RECEIVED;
	return NEED_MORE;
}

/**
 * Process a Server Key Exchange
 */
static status_t process_key_exchange(private_tls_peer_t *this,
									 bio_reader_t *reader)
{
	key_exchange_method_t group;

	this->crypto->append_handshake(this->crypto,
								TLS_SERVER_KEY_EXCHANGE, reader->peek(reader));

	group = this->crypto->get_dh_group(this->crypto);
	if (group == KE_NONE)
	{
		DBG1(DBG_TLS, "received Server Key Exchange, but not required "
			 "for current suite");
		this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
		return NEED_MORE;
	}
	if (key_exchange_is_ecdh(group))
	{
		return process_ec_key_exchange(this, reader);
	}
	return process_modp_key_exchange(this, reader);
}

/**
 * Read all available certificate authorities from the given reader
 */
static bool read_certificate_authorities(private_tls_peer_t *this,
										 bio_reader_t *reader)
{
	chunk_t data;
	bio_reader_t *authorities;
	identification_t *id;
	certificate_t *cert;

	if (!reader->read_data16(reader, &data))
	{
		return FALSE;
	}
	authorities = bio_reader_create(data);
	while (authorities->remaining(authorities))
	{
		if (!authorities->read_data16(authorities, &data))
		{
			authorities->destroy(authorities);
			return FALSE;
		}
		if (this->peer)
		{
			id = identification_create_from_encoding(ID_DER_ASN1_DN, data);
			cert = lib->credmgr->get_cert(lib->credmgr,
										  CERT_X509, KEY_ANY, id, TRUE);
			if (cert)
			{
				DBG1(DBG_TLS, "received TLS cert request for '%Y", id);
				this->peer_auth->add(this->peer_auth, AUTH_RULE_CA_CERT, cert);
			}
			else
			{
				DBG1(DBG_TLS, "received TLS cert request for unknown CA '%Y'", id);
			}
			id->destroy(id);
		}
	}
	authorities->destroy(authorities);
	return TRUE;
}

/**
 * Process a Certificate Request message
 */
static status_t process_certreq(private_tls_peer_t *this, bio_reader_t *reader)
{
	chunk_t types, hashsig, context, ext;
	bio_reader_t *extensions, *extension;

	if (!this->peer)
	{
		DBG1(DBG_TLS, "server requested a certificate, but client "
			 "authentication disabled");
	}
	this->crypto->append_handshake(this->crypto,
								TLS_CERTIFICATE_REQUEST, reader->peek(reader));

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (!reader->read_data8(reader, &types))
		{
			DBG1(DBG_TLS, "certreq message header invalid");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
		this->cert_types = chunk_clone(types);
		if (this->tls->get_version_max(this->tls) >= TLS_1_2)
		{
			if (!reader->read_data16(reader, &hashsig))
			{
				DBG1(DBG_TLS, "certreq message invalid");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				return NEED_MORE;
			}
			this->hashsig = chunk_clone(hashsig);
		}

		if (!read_certificate_authorities(this, reader))
		{
			DBG1(DBG_TLS, "certreq message invalid");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
	}
	else
	{
		/* certificate request context as described in RFC 8446, section 4.3.2 */
		reader->read_data8(reader, &context);
		reader->read_data16(reader, &ext);
		extensions = bio_reader_create(ext);
		while (extensions->remaining(extensions))
		{
			uint16_t extension_type;
			chunk_t extension_data;

			if (!extensions->read_uint16(extensions, &extension_type) ||
				!extensions->read_data16(extensions, &extension_data))
			{
				DBG1(DBG_TLS, "invalid extension in CertificateRequest");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				extensions->destroy(extensions);
				return NEED_MORE;
			}
			extension = bio_reader_create(extension_data);
			switch (extension_type)
			{
				case TLS_EXT_SIGNATURE_ALGORITHMS:
					if (!extension->read_data16(extension, &extension_data))
					{
						DBG1(DBG_TLS, "invalid %N extension",
							 tls_extension_names, extension_type);
						this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
						extension->destroy(extension);
						extensions->destroy(extensions);
						return NEED_MORE;
					}
					chunk_free(&this->hashsig);
					this->hashsig = chunk_clone(extension_data);
					break;
				case TLS_EXT_CERTIFICATE_AUTHORITIES:
					if (!read_certificate_authorities(this, extension))
					{
						DBG1(DBG_TLS, "certificate request message invalid");
						this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
						extension->destroy(extension);
						extensions->destroy(extensions);
						return NEED_MORE;
					}
					break;
				default:
					break;
			}
			extension->destroy(extension);
		}
	extensions->destroy(extensions);
	}
	this->certreq_received = TRUE;
	this->state = STATE_CERTREQ_RECEIVED;
	return NEED_MORE;
}

/**
 * Process Hello Done message
 */
static status_t process_hello_done(private_tls_peer_t *this,
								   bio_reader_t *reader)
{
	this->crypto->append_handshake(this->crypto,
								   TLS_SERVER_HELLO_DONE, reader->peek(reader));
	this->state = STATE_HELLO_DONE;
	return NEED_MORE;
}

/**
 * Process finished message
 */
static status_t process_finished(private_tls_peer_t *this, bio_reader_t *reader)
{
	chunk_t received, verify_data;
	u_char buf[12];

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (!reader->read_data(reader, sizeof(buf), &received))
		{
			DBG1(DBG_TLS, "received server finished too short");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
		if (!this->crypto->calculate_finished_legacy(this->crypto,
													 "server finished", buf))
		{
			DBG1(DBG_TLS, "calculating server finished failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		verify_data = chunk_from_thing(buf);
	}
	else
	{
		received = reader->peek(reader);
		if (!this->crypto->calculate_finished(this->crypto, TRUE, &verify_data))
		{
			DBG1(DBG_TLS, "calculating server finished failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
	}

	if (!chunk_equals_const(received, verify_data))
	{
		DBG1(DBG_TLS, "received server finished invalid");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECRYPT_ERROR);
		return NEED_MORE;
	}

	if (verify_data.ptr != buf)
	{
		chunk_free(&verify_data);
	}

	this->crypto->append_handshake(this->crypto, TLS_FINISHED, received);
	this->state = STATE_FINISHED_RECEIVED;
	return NEED_MORE;
}

/**
 * Process NewSessionTicket message
 */
static status_t process_new_session_ticket(private_tls_peer_t *this,
										   bio_reader_t *reader)
{
	uint32_t ticket_lifetime, ticket_age_add;
	chunk_t ticket_nonce, ticket, extensions;

	if (!reader->read_uint32(reader, &ticket_lifetime) ||
		!reader->read_uint32(reader, &ticket_age_add) ||
		!reader->read_data8(reader, &ticket_nonce) ||
		!reader->read_data16(reader, &ticket) ||
		!reader->read_data16(reader, &extensions))
	{
		DBG1(DBG_TLS, "received invalid NewSessionTicket");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	return NEED_MORE;
}

/**
 * Process KeyUpdate message
 */
static status_t process_key_update(private_tls_peer_t *this,
								   bio_reader_t *reader)
{
	uint8_t update_requested;

	if (!reader->read_uint8(reader, &update_requested) ||
		update_requested > 1)
	{
		DBG1(DBG_TLS, "received invalid KeyUpdate");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	if (!this->crypto->update_app_keys(this->crypto, TRUE))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	this->crypto->change_cipher(this->crypto, TRUE);

	if (update_requested)
	{
		DBG1(DBG_TLS, "server requested KeyUpdate");
		this->state = STATE_KEY_UPDATE_REQUESTED;
	}
	return NEED_MORE;
}

METHOD(tls_handshake_t, process, status_t,
	private_tls_peer_t *this, tls_handshake_type_t type, bio_reader_t *reader)
{
	tls_handshake_type_t expected DBG_UNUSED;

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		switch (this->state)
		{
			case STATE_HELLO_SENT:
				if (type == TLS_SERVER_HELLO)
				{
					return process_server_hello(this, reader);
				}
				expected = TLS_SERVER_HELLO;
				break;
			case STATE_HELLO_RECEIVED:
				if (type == TLS_CERTIFICATE)
				{
					return process_certificate(this, reader);
				}
				expected = TLS_CERTIFICATE;
				break;
			case STATE_CERT_RECEIVED:
				if (type == TLS_SERVER_KEY_EXCHANGE)
				{
					return process_key_exchange(this, reader);
				}
				/* fall through since TLS_SERVER_KEY_EXCHANGE is optional */
			case STATE_KEY_EXCHANGE_RECEIVED:
				if (type == TLS_CERTIFICATE_REQUEST)
				{
					return process_certreq(this, reader);
				}
				/* no cert request, server does not want to authenticate us */
				DESTROY_IF(this->peer);
				this->peer = NULL;
				/* fall through since TLS_CERTIFICATE_REQUEST is optional */
			case STATE_CERTREQ_RECEIVED:
				if (type == TLS_SERVER_HELLO_DONE)
				{
					return process_hello_done(this, reader);
				}
				expected = TLS_SERVER_HELLO_DONE;
				break;
			case STATE_CIPHERSPEC_CHANGED_IN:
				if (type == TLS_FINISHED)
				{
					return process_finished(this, reader);
				}
				expected = TLS_FINISHED;
				break;
			default:
				DBG1(DBG_TLS, "TLS %N not expected in current state",
					 tls_handshake_type_names, type);
				this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
				return NEED_MORE;
		}
	}
	else
	{
		switch (this->state)
		{
			case STATE_HELLO_SENT:
				if (type == TLS_SERVER_HELLO)
				{
					return process_server_hello(this, reader);
				}
				expected = TLS_SERVER_HELLO;
				break;
			case STATE_CIPHERSPEC_CHANGED_IN:
			case STATE_HELLO_RECEIVED:
				if (type == TLS_ENCRYPTED_EXTENSIONS)
				{
					return process_encrypted_extensions(this, reader);
				}
				expected = TLS_ENCRYPTED_EXTENSIONS;
				break;
			case STATE_ENCRYPTED_EXTENSIONS_RECEIVED:
				if (type == TLS_CERTIFICATE_REQUEST)
				{
					return process_certreq(this, reader);
				}
				/* no cert request, server does not want to authenticate us */
				DESTROY_IF(this->peer);
				this->peer = NULL;
				/* otherwise fall through to next state */
			case STATE_CERTREQ_RECEIVED:
				if (type == TLS_CERTIFICATE)
				{
					return process_certificate(this, reader);
				}
				expected = TLS_CERTIFICATE;
				break;
			case STATE_CERT_RECEIVED:
				if (type == TLS_CERTIFICATE_VERIFY)
				{
					return process_cert_verify(this, reader);
				}
				expected = TLS_CERTIFICATE_VERIFY;
				break;
			case STATE_CERT_VERIFY_RECEIVED:
				if (type == TLS_FINISHED)
				{
					return process_finished(this, reader);
				}
				expected = TLS_FINISHED;
				break;
			case STATE_FINISHED_RECEIVED:
				return NEED_MORE;
			case STATE_FINISHED_SENT_KEY_SWITCHED:
				if (type == TLS_NEW_SESSION_TICKET)
				{
					return process_new_session_ticket(this, reader);
				}
				if (type == TLS_KEY_UPDATE)
				{
					return process_key_update(this, reader);
				}
				expected = TLS_NEW_SESSION_TICKET;
				break;
			default:
				DBG1(DBG_TLS, "TLS %N not expected in current state",
					 tls_handshake_type_names, type);
				this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
				return NEED_MORE;
		}
	}
	DBG1(DBG_TLS, "TLS %N expected, but received %N",
		 tls_handshake_type_names, expected, tls_handshake_type_names, type);
	this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
	return NEED_MORE;
}

/**
 * Send a client hello
 */
static status_t send_client_hello(private_tls_peer_t *this,
								  tls_handshake_type_t *type,
								  bio_writer_t *writer)
{
	tls_cipher_suite_t *suites;
	bio_writer_t *extensions, *curves = NULL, *versions, *key_share, *signatures;
	tls_version_t version_max, version_min;
	key_exchange_method_t group;
	tls_named_group_t curve;
	enumerator_t *enumerator;
	int count, i, v;
	rng_t *rng;

	htoun32(&this->client_random, time(NULL));
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng ||
		!rng->get_bytes(rng, sizeof(this->client_random) - 4,
						this->client_random + 4))
	{
		DBG1(DBG_TLS, "failed to generate client random");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		DESTROY_IF(rng);
		return NEED_MORE;
	}
	rng->destroy(rng);

	/* determine supported suites before the versions as they might change */
	count = this->crypto->get_cipher_suites(this->crypto, &suites);

	/* TLS version_max in handshake protocol */
	version_max = this->tls->get_version_max(this->tls);
	version_min = this->tls->get_version_min(this->tls);
	if (version_max < TLS_1_3)
	{
		this->hello_version = version_max;
	}
	else
	{
		this->hello_version = TLS_1_2;
	}
	writer->write_uint16(writer, this->hello_version);
	writer->write_data(writer, chunk_from_thing(this->client_random));

	/* session identifier */
	this->session = this->crypto->get_session(this->crypto, this->server);
	writer->write_data8(writer, this->session);

	/* add TLS cipher suites */
	if (count <= 0)
	{
		DBG1(DBG_TLS, "no supported TLS cipher suite available");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	writer->write_uint16(writer, count * 2);
	for (i = 0; i < count; i++)
	{
		writer->write_uint16(writer, suites[i]);
	}

	/* NULL compression only */
	writer->write_uint8(writer, 1);
	writer->write_uint8(writer, 0);

	extensions = bio_writer_create(32);

	if (this->server->get_type(this->server) == ID_FQDN)
	{
		bio_writer_t *names;

		DBG2(DBG_TLS, "sending extension: Server Name Indication for '%Y'",
			 this->server);
		names = bio_writer_create(8);
		names->write_uint8(names, TLS_NAME_TYPE_HOST_NAME);
		names->write_data16(names, this->server->get_encoding(this->server));
		names->wrap16(names);
		extensions->write_uint16(extensions, TLS_EXT_SERVER_NAME);
		extensions->write_data16(extensions, names->get_buf(names));
		names->destroy(names);
	}

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, &group, &curve))
	{
		if (this->requested_curve && this->requested_curve != curve)
		{
			continue;
		}
		if (!curves)
		{
			extensions->write_uint16(extensions, TLS_EXT_SUPPORTED_GROUPS);
			curves = bio_writer_create(16);
		}
		if (!this->dh)
		{
			this->dh = lib->crypto->create_ke(lib->crypto, group);
			if (!this->dh)
			{
				continue;
			}
		}
		curves->write_uint16(curves, curve);
	}
	enumerator->destroy(enumerator);

	if (curves)
	{
		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_SUPPORTED_GROUPS);

		curves->wrap16(curves);
		extensions->write_data16(extensions, curves->get_buf(curves));
		curves->destroy(curves);

		/* if we support curves, add point format extension */
		extensions->write_uint16(extensions, TLS_EXT_EC_POINT_FORMATS);
		extensions->write_uint16(extensions, 2);
		extensions->write_uint8(extensions, 1);
		extensions->write_uint8(extensions, TLS_EC_POINT_UNCOMPRESSED);
	}

	if (version_max >= TLS_1_3)
	{
		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_SUPPORTED_VERSIONS);
		extensions->write_uint16(extensions, TLS_EXT_SUPPORTED_VERSIONS);
		versions = bio_writer_create(0);
		for (v = version_max; v >= version_min; v--)
		{
			versions->write_uint16(versions, v);
		}
		versions->wrap8(versions);
		extensions->write_data16(extensions, versions->get_buf(versions));
		versions->destroy(versions);
	}

	if (this->cookie.len)
	{
		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_COOKIE);
		extensions->write_uint16(extensions, TLS_EXT_COOKIE);
		extensions->write_uint16(extensions, this->cookie.len + 2);
		extensions->write_data16(extensions, this->cookie);
		chunk_free(&this->cookie);
	}

	DBG2(DBG_TLS, "sending extension: %N",
		 tls_extension_names, TLS_EXT_SIGNATURE_ALGORITHMS);
	extensions->write_uint16(extensions, TLS_EXT_SIGNATURE_ALGORITHMS);
	signatures = bio_writer_create(32);
	this->crypto->get_signature_algorithms(this->crypto, signatures, FALSE);
	extensions->write_data16(extensions, signatures->get_buf(signatures));
	signatures->destroy(signatures);

	DBG2(DBG_TLS, "sending extension: %N",
		 tls_extension_names, TLS_EXT_SIGNATURE_ALGORITHMS_CERT);
	extensions->write_uint16(extensions, TLS_EXT_SIGNATURE_ALGORITHMS_CERT);
	signatures = bio_writer_create(32);
	this->crypto->get_signature_algorithms(this->crypto, signatures, TRUE);
	extensions->write_data16(extensions, signatures->get_buf(signatures));
	signatures->destroy(signatures);

	if (this->tls->get_version_max(this->tls) >= TLS_1_3)
	{
		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_KEY_SHARE);
		extensions->write_uint16(extensions, TLS_EXT_KEY_SHARE);
		if (!tls_write_key_share(&key_share, this->dh))
		{
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			extensions->destroy(extensions);
			return NEED_MORE;
		}
		key_share->wrap16(key_share);
		extensions->write_data16(extensions, key_share->get_buf(key_share));
		key_share->destroy(key_share);
	}

	writer->write_data16(writer, extensions->get_buf(extensions));
	extensions->destroy(extensions);

	*type = TLS_CLIENT_HELLO;
	this->state = STATE_HELLO_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Convert certificate types to signature schemes so TLS version <= 1.1 can use
 * the same private key enumeration as newer TLS versions.
 */
static void convert_cert_types(private_tls_peer_t *this)
{
	bio_reader_t *reader;
	bio_writer_t *writer;
	uint8_t type;

	reader = bio_reader_create(this->cert_types);
	writer = bio_writer_create(0);
	while (reader->remaining(reader) && reader->read_uint8(reader, &type))
	{
		/* each certificate type is mapped to one signature scheme, which is not
		 * ideal but serves our needs in legacy TLS versions */
		switch (type)
		{
			case TLS_RSA_SIGN:
				writer->write_uint16(writer, TLS_SIG_RSA_PKCS1_SHA256);
				break;
			case TLS_ECDSA_SIGN:
				writer->write_uint16(writer, TLS_SIG_ECDSA_SHA256);
				break;
			default:
				continue;
		}
	}
	reader->destroy(reader);
	this->hashsig = writer->extract_buf(writer);
	writer->destroy(writer);
}

/**
 * Send Certificate
 */
static status_t send_certificate(private_tls_peer_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	auth_rule_t rule;
	bio_writer_t *certs;
	private_key_t *key;
	auth_cfg_t *auth;
	chunk_t data;
	tls_version_t version_min, version_max;

	version_min = this->tls->get_version_min(this->tls);
	version_max = this->tls->get_version_max(this->tls);

	if (this->peer)
	{
		if (!this->hashsig.len)
		{
			convert_cert_types(this);
		}
		enumerator = tls_create_private_key_enumerator(version_min, version_max,
													   this->hashsig, this->peer);
		if (!enumerator || !enumerator->enumerate(enumerator, &key, &auth))
		{
			if (!enumerator)
			{
				DBG1(DBG_TLS, "no common signature algorithms found");
			}
			else
			{
				DBG1(DBG_TLS, "no usable TLS client certificate found for '%Y'",
					 this->peer);
			}
			this->peer->destroy(this->peer);
			this->peer = NULL;
		}
		else
		{
			this->private = key->get_ref(key);
			this->peer_auth->merge(this->peer_auth, auth, FALSE);
		}
		DESTROY_IF(enumerator);
	}

	/* certificate request context as described in RFC 8446, section 4.4.2 */
	if (version_max > TLS_1_2)
	{
		writer->write_uint8(writer, 0);
	}

	/* generate certificate payload */
	certs = bio_writer_create(256);
	cert = this->peer_auth->get(this->peer_auth, AUTH_RULE_SUBJECT_CERT);
	if (cert)
	{
		if (cert->get_encoding(cert, CERT_ASN1_DER, &data))
		{
			DBG1(DBG_TLS, "sending TLS client certificate '%Y'",
				 cert->get_subject(cert));
			certs->write_data24(certs, data);
			free(data.ptr);

			/* extensions see RFC 8446, section 4.4.2 */
			if (version_max > TLS_1_2)
			{
				certs->write_uint16(certs, 0);
			}
		}
	}
	enumerator = this->peer_auth->create_enumerator(this->peer_auth);
	while (enumerator->enumerate(enumerator, &rule, &cert))
	{
		if (rule == AUTH_RULE_IM_CERT)
		{
			if (cert->get_encoding(cert, CERT_ASN1_DER, &data))
			{
				DBG1(DBG_TLS, "sending TLS intermediate certificate '%Y'",
					 cert->get_subject(cert));
				certs->write_data24(certs, data);
				free(data.ptr);

				/* extensions see RFC 8446, section 4.4.2 */
				if (version_max > TLS_1_2)
				{
					certs->write_uint16(certs, 0);
				}
			}
		}
	}
	enumerator->destroy(enumerator);

	writer->write_data24(writer, certs->get_buf(certs));
	certs->destroy(certs);

	*type = TLS_CERTIFICATE;
	this->state = STATE_CERT_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send client key exchange, using premaster encryption
 */
static status_t send_key_exchange_encrypt(private_tls_peer_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	public_key_t *public;
	rng_t *rng;
	char premaster[48];
	chunk_t encrypted;

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng || !rng->get_bytes(rng, sizeof(premaster) - 2, premaster + 2))
	{
		DBG1(DBG_TLS, "failed to generate TLS premaster secret");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		DESTROY_IF(rng);
		return NEED_MORE;
	}
	rng->destroy(rng);
	htoun16(premaster, this->hello_version);

	if (!this->crypto->derive_secrets(this->crypto, chunk_from_thing(premaster),
									  this->session, this->server,
									  chunk_from_thing(this->client_random),
									  chunk_from_thing(this->server_random)))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	public = tls_find_public_key(this->server_auth, this->server);
	if (!public)
	{
		DBG1(DBG_TLS, "no TLS public key found for server '%Y'", this->server);
		this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
		return NEED_MORE;
	}
	if (!public->encrypt(public, ENCRYPT_RSA_PKCS1, NULL,
						 chunk_from_thing(premaster), &encrypted))
	{
		public->destroy(public);
		DBG1(DBG_TLS, "encrypting TLS premaster secret failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_BAD_CERTIFICATE);
		return NEED_MORE;
	}
	public->destroy(public);

	writer->write_data16(writer, encrypted);
	free(encrypted.ptr);

	*type = TLS_CLIENT_KEY_EXCHANGE;
	this->state = STATE_KEY_EXCHANGE_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send client key exchange, using DHE exchange
 */
static status_t send_key_exchange_dhe(private_tls_peer_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	chunk_t premaster, pub;

	if (!this->dh->get_shared_secret(this->dh, &premaster))
	{
		DBG1(DBG_TLS, "calculating premaster from DH failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (!this->crypto->derive_secrets(this->crypto, premaster,
									  this->session, this->server,
									  chunk_from_thing(this->client_random),
									  chunk_from_thing(this->server_random)))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		chunk_clear(&premaster);
		return NEED_MORE;
	}
	chunk_clear(&premaster);

	if (!this->dh->get_public_key(this->dh, &pub))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	switch (this->dh->get_method(this->dh))
	{
		case MODP_CUSTOM:
			writer->write_data16(writer, pub);
			break;
		case CURVE_25519:
		case CURVE_448:
			/* ECPoint uses an 8-bit length header only */
			writer->write_data8(writer, pub);
			break;
		default:
			/* classic ECPoint format (see RFC 8422, section 5.4.1) */
			writer->write_uint8(writer, pub.len + 1);
			writer->write_uint8(writer, TLS_ANSI_UNCOMPRESSED);
			writer->write_data(writer, pub);
			break;
	}
	free(pub.ptr);

	*type = TLS_CLIENT_KEY_EXCHANGE;
	this->state = STATE_KEY_EXCHANGE_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send client key exchange, depending on suite
 */
static status_t send_key_exchange(private_tls_peer_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	if (this->dh)
	{
		return send_key_exchange_dhe(this, type, writer);
	}
	return send_key_exchange_encrypt(this, type, writer);
}

/**
 * Send certificate verify
 */
static status_t send_certificate_verify(private_tls_peer_t *this,
										tls_handshake_type_t *type,
										bio_writer_t *writer)
{
	if (!this->private ||
		!this->crypto->sign_handshake(this->crypto, this->private,
									  writer, this->hashsig))
	{
		DBG1(DBG_TLS, "creating TLS Certificate Verify signature failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	*type = TLS_CERTIFICATE_VERIFY;
	this->state = STATE_VERIFY_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send Finished
 */
static status_t send_finished(private_tls_peer_t *this,
							  tls_handshake_type_t *type, bio_writer_t *writer)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		char buf[12];

		if (!this->crypto->calculate_finished_legacy(this->crypto,
													 "client finished", buf))
		{
			DBG1(DBG_TLS, "calculating client finished data failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}

		writer->write_data(writer, chunk_from_thing(buf));
	}
	else
	{
		chunk_t verify_data;

		if (!this->crypto->calculate_finished(this->crypto, FALSE, &verify_data))
		{
			DBG1(DBG_TLS, "calculating client finished data failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}

		writer->write_data(writer, verify_data);
		chunk_free(&verify_data);
	}

	*type = TLS_FINISHED;
	this->state = STATE_FINISHED_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send KeyUpdate message
 */
static status_t send_key_update(private_tls_peer_t *this,
								tls_handshake_type_t *type, bio_writer_t *writer)
{
	*type = TLS_KEY_UPDATE;

	/* we currently only send this as reply, so we never request an update */
	writer->write_uint8(writer, 0);

	this->state = STATE_KEY_UPDATE_SENT;
	return NEED_MORE;
}

METHOD(tls_handshake_t, build, status_t,
	private_tls_peer_t *this, tls_handshake_type_t *type, bio_writer_t *writer)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		switch (this->state)
		{
			case STATE_INIT:
				return send_client_hello(this, type, writer);
			case STATE_HELLO_DONE:
				if (this->peer || this->certreq_received)
				{
					return send_certificate(this, type, writer);
				}
				/* otherwise fall through to next state */
			case STATE_CERT_SENT:
				return send_key_exchange(this, type, writer);
			case STATE_KEY_EXCHANGE_SENT:
				if (this->peer)
				{
					return send_certificate_verify(this, type, writer);
				}
				else
				{
					return INVALID_STATE;
				}
			case STATE_CIPHERSPEC_CHANGED_OUT:
				return send_finished(this, type, writer);
			default:
				return INVALID_STATE;
		}
	}
	else
	{
		switch (this->state)
		{
			case STATE_INIT:
				return send_client_hello(this, type, writer);
			case STATE_HELLO_DONE:
			case STATE_CIPHERSPEC_CHANGED_OUT:
			case STATE_FINISHED_RECEIVED:
				if (!this->crypto->derive_app_keys(this->crypto))
				{
					this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
					return NEED_MORE;
				}
				this->crypto->change_cipher(this->crypto, TRUE);
				if (this->peer || this->certreq_received)
				{
					return send_certificate(this, type, writer);
				}
				/* otherwise fall through to next state */
			case STATE_CERT_SENT:
				if (this->peer)
				{
					return send_certificate_verify(this, type, writer);
				}
				/* otherwise fall through to next state */
			case STATE_VERIFY_SENT:
				return send_finished(this, type, writer);
			case STATE_FINISHED_SENT:
				this->crypto->change_cipher(this->crypto, FALSE);
				this->state = STATE_FINISHED_SENT_KEY_SWITCHED;
				return INVALID_STATE;
			case STATE_KEY_UPDATE_REQUESTED:
				return send_key_update(this, type, writer);
			case STATE_KEY_UPDATE_SENT:
				if (!this->crypto->update_app_keys(this->crypto, FALSE))
				{
					this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
					return NEED_MORE;
				}
				this->crypto->change_cipher(this->crypto, FALSE);
				this->state = STATE_FINISHED_SENT_KEY_SWITCHED;
				return INVALID_STATE;
			default:
				return INVALID_STATE;
		}
	}
}

/**
 * Check if we are currently retrying to connect to the server.
 */
static bool retrying(private_tls_peer_t *this)
{
	return this->state == STATE_INIT && (this->requested_curve || this->cookie.len);
}

METHOD(tls_handshake_t, cipherspec_changed, bool,
	private_tls_peer_t *this, bool inbound)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (inbound)
		{
			if (this->resume)
			{
				return this->state == STATE_HELLO_RECEIVED;
			}
			return this->state == STATE_FINISHED_SENT;
		}
		else
		{
			if (this->resume)
			{
				return this->state == STATE_FINISHED_RECEIVED;
			}
			if (this->peer)
			{
				return this->state == STATE_VERIFY_SENT;
			}
			return this->state == STATE_KEY_EXCHANGE_SENT;

		}
	}
	else
	{
		if (inbound)
		{	/* accept ChangeCipherSpec after ServerHello or HelloRetryRequest */
			return this->state == STATE_HELLO_RECEIVED || retrying(this);
		}
		else
		{
			return FALSE;
		}
	}
}

METHOD(tls_handshake_t, change_cipherspec, void,
	private_tls_peer_t *this, bool inbound)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		this->crypto->change_cipher(this->crypto, inbound);
	}

	if (retrying(this))
	{	/* servers might send a ChangeCipherSpec after a HelloRetryRequest,
		 * which should not cause any state changes */
		return;
	}

	if (inbound)
	{
		this->state = STATE_CIPHERSPEC_CHANGED_IN;
	}
	else
	{
		this->state = STATE_CIPHERSPEC_CHANGED_OUT;
	}
}

METHOD(tls_handshake_t, finished, bool,
	private_tls_peer_t *this)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (this->resume)
		{
			return this->state == STATE_FINISHED_SENT;
		}
		return this->state == STATE_FINISHED_RECEIVED;
	}
	else
	{
		return this->state == STATE_FINISHED_SENT_KEY_SWITCHED;
	}
}

METHOD(tls_handshake_t, get_peer_id, identification_t*,
	private_tls_peer_t *this)
{
	return this->peer;
}

METHOD(tls_handshake_t, get_server_id, identification_t*,
	private_tls_peer_t *this)
{
	return this->server;
}

METHOD(tls_handshake_t, get_auth, auth_cfg_t*,
	private_tls_peer_t *this)
{
	return this->server_auth;
}

METHOD(tls_handshake_t, destroy, void,
	private_tls_peer_t *this)
{
	DESTROY_IF(this->private);
	DESTROY_IF(this->dh);
	DESTROY_IF(this->peer);
	this->server->destroy(this->server);
	this->peer_auth->destroy(this->peer_auth);
	this->server_auth->destroy(this->server_auth);
	free(this->hashsig.ptr);
	free(this->cert_types.ptr);
	free(this->session.ptr);
	free(this->cookie.ptr);
	free(this);
}

/**
 * See header
 */
tls_peer_t *tls_peer_create(tls_t *tls, tls_crypto_t *crypto, tls_alert_t *alert,
							identification_t *peer, identification_t *server)
{
	private_tls_peer_t *this;

	INIT(this,
		.public = {
			.handshake = {
				.process = _process,
				.build = _build,
				.cipherspec_changed = _cipherspec_changed,
				.change_cipherspec = _change_cipherspec,
				.finished = _finished,
				.get_peer_id = _get_peer_id,
				.get_server_id = _get_server_id,
				.get_auth = _get_auth,
				.destroy = _destroy,
			},
		},
		.state = STATE_INIT,
		.tls = tls,
		.crypto = crypto,
		.alert = alert,
		.peer = peer ? peer->clone(peer) : NULL,
		.server = server->clone(server),
		.peer_auth = auth_cfg_create(),
		.server_auth = auth_cfg_create(),
	);

	return &this->public;
}

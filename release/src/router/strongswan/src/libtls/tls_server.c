/*
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

#include "tls_server.h"

#include <time.h>

#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <collections/array.h>

typedef struct private_tls_server_t private_tls_server_t;

/**
 * Size of a session ID
 */
#define SESSION_ID_SIZE 16

typedef enum {
	STATE_INIT,
	STATE_HELLO_RECEIVED,
	STATE_HELLO_SENT,
	STATE_CERT_SENT,
	STATE_KEY_EXCHANGE_SENT,
	STATE_CERTREQ_SENT,
	STATE_HELLO_DONE,
	STATE_CERT_RECEIVED,
	STATE_KEY_EXCHANGE_RECEIVED,
	STATE_CERT_VERIFY_RECEIVED,
	STATE_CIPHERSPEC_CHANGED_IN,
	STATE_FINISHED_RECEIVED,
	STATE_CIPHERSPEC_CHANGED_OUT,
	STATE_FINISHED_SENT,
	/* new states in TLS 1.3 */
	STATE_ENCRYPTED_EXTENSIONS_SENT,
	STATE_CERT_VERIFY_SENT,
	STATE_KEY_UPDATE_REQUESTED,
	STATE_KEY_UPDATE_SENT,
	STATE_FINISHED_SENT_KEY_SWITCHED,
} server_state_t;

/**
 * Private data of an tls_server_t object.
 */
struct private_tls_server_t {

	/**
	 * Public tls_server_t interface.
	 */
	tls_server_t public;

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
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity, NULL for no client authentication
	 */
	identification_t *peer;

	/**
	 * State we are in
	 */
	server_state_t state;

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
	 * Selected TLS cipher suite
	 */
	tls_cipher_suite_t suite;

	/**
	 * Offered TLS version of the client
	 */
	tls_version_t client_version;

	/**
	 * TLS session identifier
	 */
	chunk_t session;

	/**
	 * Do we resume a session?
	 */
	bool resume;

	/**
	 * Hash and signature algorithms supported by peer
	 */
	chunk_t hashsig;

	/**
	 * Elliptic curves supported by peer
	 */
	chunk_t curves;

	/**
	 * Did we receive the curves from the client?
	 */
	bool curves_received;

	/**
	 * Whether to include CAs in CertificateRequest messages
	 */
	bool send_certreq_authorities;
};

/**
 * Find a trusted public key to encrypt/verify key exchange data
 */
public_key_t *tls_find_public_key(auth_cfg_t *peer_auth, identification_t *id)
{
	public_key_t *public = NULL, *current;
	certificate_t *cert, *found;
	key_type_t key_type = KEY_ANY;
	enumerator_t *enumerator;
	auth_cfg_t *auth;

	cert = peer_auth->get(peer_auth, AUTH_HELPER_SUBJECT_CERT);
	if (cert)
	{
		current = cert->get_public_key(cert);
		if (current)
		{
			key_type = current->get_type(current);
			current->destroy(current);
		}
		enumerator = lib->credmgr->create_public_enumerator(lib->credmgr,
											key_type, id, peer_auth, TRUE);
		while (enumerator->enumerate(enumerator, &current, &auth))
		{
			found = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
			if (found && cert->equals(cert, found))
			{
				public = current->get_ref(current);
				peer_auth->merge(peer_auth, auth, FALSE);
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	return public;
}

/**
 * Find a cipher suite and a server key
 */
static bool select_suite_and_key(private_tls_server_t *this,
								 tls_cipher_suite_t *suites, int count)
{
	tls_version_t version_min, version_max;
	private_key_t *key;
	auth_cfg_t *auth;
	enumerator_t *enumerator;

	version_min = this->tls->get_version_min(this->tls);
	version_max = this->tls->get_version_max(this->tls);
	enumerator = tls_create_private_key_enumerator(version_min, version_max,
												   this->hashsig, this->server);
	if (!enumerator)
	{
		DBG1(DBG_TLS, "no common signature algorithms found");
		return FALSE;
	}
	if (!enumerator->enumerate(enumerator, &key, &auth))
	{
		DBG1(DBG_TLS, "no usable TLS server certificate found for '%Y'",
			 this->server);
		enumerator->destroy(enumerator);
		return FALSE;
	}

	if (version_max >= TLS_1_3)
	{
		this->suite = this->crypto->select_cipher_suite(this->crypto, suites,
												  		count, KEY_ANY);
	}
	else
	{
		this->suite = this->crypto->select_cipher_suite(this->crypto, suites,
														count, key->get_type(key));
		while (!this->suite &&
			   enumerator->enumerate(enumerator, &key, &auth))
		{	/* find a key and cipher suite for one of the remaining key types */
			this->suite = this->crypto->select_cipher_suite(this->crypto,
															suites, count,
															key->get_type(key));
		}
	}
	if (!this->suite)
	{
		DBG1(DBG_TLS, "received cipher suites or signature schemes unacceptable");
		enumerator->destroy(enumerator);
		return FALSE;
	}
	DBG1(DBG_TLS, "using key of type %N", key_type_names, key->get_type(key));
	DESTROY_IF(this->private);
	this->private = key->get_ref(key);
	this->server_auth->purge(this->server_auth, FALSE);
	this->server_auth->merge(this->server_auth, auth, FALSE);
	enumerator->destroy(enumerator);
	return TRUE;
}

/**
 * Check if the peer supports a given TLS curve
 */
static bool peer_supports_curve(private_tls_server_t *this,
								tls_named_group_t curve)
{
	bio_reader_t *reader;
	uint16_t current;

	if (!this->curves_received)
	{	/* none received, assume yes */
		return TRUE;
	}
	reader = bio_reader_create(this->curves);
	while (reader->remaining(reader) && reader->read_uint16(reader, &current))
	{
		if (current == curve)
		{
			reader->destroy(reader);
			return TRUE;
		}
	}
	reader->destroy(reader);
	return FALSE;
}

/**
 * TLS 1.3 key exchange key share
 */
typedef struct {
	uint16_t curve;
	chunk_t key_share;
} key_share_t;

/**
 * Check if peer sent a key share of a given TLS named DH group
 */
static bool peer_offered_curve(array_t *key_shares, tls_named_group_t curve,
							   key_share_t *out)
{
	key_share_t peer;
	int i;

	for (i = 0; i < array_count(key_shares); i++)
	{
		array_get(key_shares, i, &peer);
		if (curve == peer.curve)
		{
			if (out)
			{
				*out = peer;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Check if client is currently retrying to connect to the server.
 */
static bool retrying(private_tls_server_t *this)
{
	return this->state == STATE_INIT && this->requested_curve;
}

/**
 * Process client hello message
 */
static status_t process_client_hello(private_tls_server_t *this,
									 bio_reader_t *reader)
{
	uint16_t legacy_version = 0, version = 0, extension_type = 0;
	chunk_t random, session, ciphers, versions = chunk_empty, compression;
	chunk_t ext = chunk_empty, key_shares = chunk_empty;
	key_share_t peer = {0};
	chunk_t extension_data = chunk_empty;
	bio_reader_t *extensions, *extension;
	tls_cipher_suite_t *suites;
	tls_version_t original_version_max;
	int count, i;
	rng_t *rng;

	this->crypto->append_handshake(this->crypto,
								   TLS_CLIENT_HELLO, reader->peek(reader));

	if (!reader->read_uint16(reader, &legacy_version) ||
		!reader->read_data(reader, sizeof(this->client_random), &random) ||
		!reader->read_data8(reader, &session) ||
		!reader->read_data16(reader, &ciphers) ||
		!reader->read_data8(reader, &compression) ||
		(reader->remaining(reader) && !reader->read_data16(reader, &ext)))
	{
		DBG1(DBG_TLS, "received invalid ClientHello");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	/* before we do anything version-related, determine our supported suites
	 * as that might change the min./max. versions */
	this->crypto->get_cipher_suites(this->crypto, NULL);

	extensions = bio_reader_create(ext);
	while (extensions->remaining(extensions))
	{
		if (!extensions->read_uint16(extensions, &extension_type) ||
			!extensions->read_data16(extensions, &extension_data))
		{
			DBG1(DBG_TLS, "received invalid ClientHello Extensions");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			extensions->destroy(extensions);
			return NEED_MORE;
		}
		extension = bio_reader_create(extension_data);
		DBG2(DBG_TLS, "received TLS '%N' extension",
			 tls_extension_names, extension_type);
		DBG3(DBG_TLS, "%B", &extension_data);
		switch (extension_type)
		{
			case TLS_EXT_SIGNATURE_ALGORITHMS:
				if (!extension->read_data16(extension, &extension_data))
				{
					DBG1(DBG_TLS, "invalid %N extension",
						 tls_extension_names, extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				chunk_free(&this->hashsig);
				this->hashsig = chunk_clone(extension_data);
				break;
			case TLS_EXT_SUPPORTED_GROUPS:
				if (!extension->read_data16(extension, &extension_data))
				{
					DBG1(DBG_TLS, "invalid %N extension",
						 tls_extension_names, extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				chunk_free(&this->curves);
				this->curves_received = TRUE;
				this->curves = chunk_clone(extension_data);
				break;
			case TLS_EXT_SUPPORTED_VERSIONS:
				if (!extension->read_data8(extension, &versions))
				{
					DBG1(DBG_TLS, "invalid %N extension",
						 tls_extension_names, extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				break;
			case TLS_EXT_KEY_SHARE:
				if (!extension->read_data16(extension, &key_shares))
				{
					DBG1(DBG_TLS, "invalid %N extension",
						 tls_extension_names, extension_type);
					this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
					extensions->destroy(extensions);
					extension->destroy(extension);
					return NEED_MORE;
				}
				break;
			default:
				break;
		}
		extension->destroy(extension);
	}
	extensions->destroy(extensions);

	if (this->tls->get_version_max(this->tls) >= TLS_1_3 && !this->hashsig.len)
	{
		DBG1(DBG_TLS, "no %N extension received", tls_extension_names,
			 TLS_MISSING_EXTENSION);
		this->alert->add(this->alert, TLS_FATAL, TLS_MISSING_EXTENSION);
		return NEED_MORE;
	}

	memcpy(this->client_random, random.ptr, sizeof(this->client_random));

	htoun32(&this->server_random, time(NULL));
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng ||
		!rng->get_bytes(rng, sizeof(this->server_random) - 4,
						this->server_random + 4))
	{
		DBG1(DBG_TLS, "failed to generate server random");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		DESTROY_IF(rng);
		return NEED_MORE;
	}
	rng->destroy(rng);

	original_version_max = this->tls->get_version_max(this->tls);

	if (versions.len)
	{
		bio_reader_t *client_versions;

		client_versions = bio_reader_create(versions);
		while (client_versions->remaining(client_versions))
		{
			if (client_versions->read_uint16(client_versions, &version))
			{
				if (this->tls->set_version(this->tls, version, version))
				{
					this->client_version = version;
					break;
				}
			}
		}
		client_versions->destroy(client_versions);
	}
	else
	{
		version = legacy_version;
		if (this->tls->set_version(this->tls, version, version))
		{
			this->client_version = version;
		}
	}

	/* downgrade protection (see RFC 8446, section 4.1.3) */
	if ((original_version_max == TLS_1_3 && version < TLS_1_3) ||
		(original_version_max == TLS_1_2 && version < TLS_1_2))
	{
		chunk_t downgrade_protection = tls_downgrade_protection_tls11;

		if (version == TLS_1_2)
		{
			downgrade_protection = tls_downgrade_protection_tls12;
		}
		memcpy(&this->server_random[24], downgrade_protection.ptr,
			   downgrade_protection.len);
	}

	if (!this->client_version)
	{
		DBG1(DBG_TLS, "proposed version %N not supported", tls_version_names,
	   		 version);
		this->alert->add(this->alert, TLS_FATAL, TLS_PROTOCOL_VERSION);
		return NEED_MORE;
	}

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		this->suite = this->crypto->resume_session(this->crypto, session,
										 this->peer,
										 chunk_from_thing(this->client_random),
										 chunk_from_thing(this->server_random));
	}

	if (this->suite && !retrying(this))
	{
		this->session = chunk_clone(session);
		this->resume = TRUE;
		DBG1(DBG_TLS, "resumed %N using suite %N",
			 tls_version_names, this->tls->get_version_max(this->tls),
			 tls_cipher_suite_names, this->suite);
	}
	else
	{
		tls_cipher_suite_t original_suite = this->suite;

		count = ciphers.len / sizeof(uint16_t);
		suites = alloca(count * sizeof(tls_cipher_suite_t));
		DBG2(DBG_TLS, "received %d TLS cipher suites:", count);
		for (i = 0; i < count; i++)
		{
			suites[i] = untoh16(&ciphers.ptr[i * sizeof(uint16_t)]);
			DBG2(DBG_TLS, "  %N", tls_cipher_suite_names, suites[i]);
		}
		if (!select_suite_and_key(this, suites, count))
		{
			this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
			return NEED_MORE;
		}
		if (retrying(this) && original_suite != this->suite)
		{
			DBG1(DBG_TLS, "selected %N instead of %N during retry",
				 tls_cipher_suite_names, this->suite, tls_cipher_suite_names,
				 original_suite);
			this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
			return NEED_MORE;
		}
		if (this->tls->get_version_max(this->tls) < TLS_1_3)
		{
			rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
			if (!rng ||
				!rng->allocate_bytes(rng, SESSION_ID_SIZE, &this->session))
			{
				DBG1(DBG_TLS, "generating TLS session identifier failed, skipped");
			}
			DESTROY_IF(rng);
		}
		else
		{
			chunk_free(&this->session);
			this->session = chunk_clone(session);
		}
		DBG1(DBG_TLS, "negotiated %N using suite %N",
			 tls_version_names, this->tls->get_version_max(this->tls),
			 tls_cipher_suite_names, this->suite);
	}

	if (this->tls->get_version_max(this->tls) >= TLS_1_3)
	{
		key_exchange_method_t group;
		tls_named_group_t curve, requesting_curve = 0;
		enumerator_t *enumerator;
		array_t *peer_key_shares;

		peer_key_shares = array_create(sizeof(key_share_t), 1);
		extension = bio_reader_create(key_shares);
		while (extension->remaining(extension))
		{
			if (!extension->read_uint16(extension, &peer.curve) ||
				!extension->read_data16(extension, &peer.key_share) ||
				!peer.key_share.len)
			{
				DBG1(DBG_TLS, "invalid %N extension",
					 tls_extension_names, extension_type);
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				extension->destroy(extension);
				array_destroy(peer_key_shares);
				return NEED_MORE;
			}
			array_insert(peer_key_shares, ARRAY_TAIL, &peer);
		}
		extension->destroy(extension);

		enumerator = this->crypto->create_ec_enumerator(this->crypto);
		while (enumerator->enumerate(enumerator, &group, &curve))
		{
			if (!requesting_curve &&
				peer_supports_curve(this, curve) &&
				!peer_offered_curve(peer_key_shares, curve, NULL))
			{
				requesting_curve = curve;
			}
			if (peer_supports_curve(this, curve) &&
				peer_offered_curve(peer_key_shares, curve, &peer))
			{
				DBG1(DBG_TLS, "using key exchange %N",
					 tls_named_group_names, curve);
				this->dh = lib->crypto->create_ke(lib->crypto, group);
				break;
			}
		}
		enumerator->destroy(enumerator);
		array_destroy(peer_key_shares);

		if (!this->dh)
		{
			if (retrying(this))
			{
				DBG1(DBG_TLS, "already replied with a hello retry request");
				this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
				return NEED_MORE;
			}

			if (!requesting_curve)
			{
				DBG1(DBG_TLS, "no mutual supported group in client hello");
				this->alert->add(this->alert, TLS_FATAL, TLS_ILLEGAL_PARAMETER);
				return NEED_MORE;
			}
			this->requested_curve = requesting_curve;

			if (!this->crypto->hash_handshake(this->crypto, NULL))
			{
				DBG1(DBG_TLS, "failed to hash handshake messages");
				this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
				return NEED_MORE;
			}
		}
		else
		{
			if (peer.key_share.len &&
				peer.curve != TLS_CURVE25519 &&
				peer.curve != TLS_CURVE448)
			{	/* classic format (see RFC 8446, section 4.2.8.2) */
				if (peer.key_share.ptr[0] != TLS_ANSI_UNCOMPRESSED)
				{
					DBG1(DBG_TLS, "DH point format '%N' not supported",
						 tls_ansi_point_format_names, peer.key_share.ptr[0]);
					this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
					return NEED_MORE;
				}
				peer.key_share = chunk_skip(peer.key_share, 1);
			}
			if (!peer.key_share.len ||
				!this->dh->set_public_key(this->dh, peer.key_share))
			{
				DBG1(DBG_TLS, "DH key derivation failed");
				this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
				return NEED_MORE;
			}
			this->requested_curve = 0;
		}
	}

	this->state = STATE_HELLO_RECEIVED;
	return NEED_MORE;
}

/**
 * Process certificate
 */
static status_t process_certificate(private_tls_server_t *this,
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
	if (!certs->remaining(certs))
	{
		if (this->tls->get_flags(this->tls) & TLS_FLAG_CLIENT_AUTH_OPTIONAL)
		{
			/* client authentication is not required so we clear the identity */
			DESTROY_IF(this->peer);
			this->peer = NULL;
		}
		else
		{
			DBG1(DBG_TLS, "no certificate sent by peer");
			this->alert->add(this->alert, TLS_FATAL,
							 this->tls->get_version_max(this->tls) > TLS_1_2 ?
							 TLS_CERTIFICATE_REQUIRED : TLS_HANDSHAKE_FAILURE);
			return NEED_MORE;
		}
	}
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
				this->peer_auth->add(this->peer_auth,
									 AUTH_HELPER_SUBJECT_CERT, cert);
				DBG1(DBG_TLS, "received TLS peer certificate '%Y'",
					 cert->get_subject(cert));
				first = FALSE;
				if (this->peer && this->peer->get_type(this->peer) == ID_ANY)
				{
					this->peer->destroy(this->peer);
					this->peer = cert->get_subject(cert);
					this->peer = this->peer->clone(this->peer);
				}
			}
			else
			{
				DBG1(DBG_TLS, "received TLS intermediate certificate '%Y'",
					 cert->get_subject(cert));
				this->peer_auth->add(this->peer_auth, AUTH_HELPER_IM_CERT, cert);
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
 * Process Client Key Exchange, using premaster encryption
 */
static status_t process_key_exchange_encrypted(private_tls_server_t *this,
											   bio_reader_t *reader)
{
	chunk_t encrypted, decrypted;
	char premaster[48];
	rng_t *rng;

	this->crypto->append_handshake(this->crypto,
								   TLS_CLIENT_KEY_EXCHANGE, reader->peek(reader));

	if (!reader->read_data16(reader, &encrypted))
	{
		DBG1(DBG_TLS, "received invalid Client Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	htoun16(premaster, this->client_version);
	/* pre-randomize premaster for failure cases */
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->get_bytes(rng, sizeof(premaster) - 2, premaster + 2))
	{
		DBG1(DBG_TLS, "failed to generate premaster secret");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		DESTROY_IF(rng);
		return NEED_MORE;
	}
	rng->destroy(rng);

	if (this->private &&
		this->private->decrypt(this->private, ENCRYPT_RSA_PKCS1, NULL,
							   encrypted, &decrypted))
	{
		if (decrypted.len == sizeof(premaster) &&
			untoh16(decrypted.ptr) == this->client_version)
		{
			memcpy(premaster + 2, decrypted.ptr + 2, sizeof(premaster) - 2);
		}
		else
		{
			DBG1(DBG_TLS, "decrypted premaster has invalid length/version");
		}
		chunk_clear(&decrypted);
	}
	else
	{
		DBG1(DBG_TLS, "decrypting Client Key Exchange failed");
	}

	if (!this->crypto->derive_secrets(this->crypto, chunk_from_thing(premaster),
									  this->session, this->peer,
									  chunk_from_thing(this->client_random),
									  chunk_from_thing(this->server_random)))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	this->state = STATE_KEY_EXCHANGE_RECEIVED;
	return NEED_MORE;
}

/**
 * Process client key exchange, using DHE exchange
 */
static status_t process_key_exchange_dhe(private_tls_server_t *this,
										 bio_reader_t *reader)
{
	chunk_t premaster, pub;
	key_exchange_method_t group;
	bool ec;

	this->crypto->append_handshake(this->crypto,
								   TLS_CLIENT_KEY_EXCHANGE, reader->peek(reader));

	group = this->dh->get_method(this->dh);
	ec = key_exchange_is_ecdh(group);
	if ((ec && !reader->read_data8(reader, &pub)) ||
		(!ec && (!reader->read_data16(reader, &pub) || pub.len == 0)))
	{
		DBG1(DBG_TLS, "received invalid Client Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	if (ec &&
		group != CURVE_25519 &&
		group != CURVE_448)
	{
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
	if (!this->dh->get_shared_secret(this->dh, &premaster))
	{
		DBG1(DBG_TLS, "calculating premaster from DH failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	if (!this->crypto->derive_secrets(this->crypto, premaster,
									  this->session, this->peer,
									  chunk_from_thing(this->client_random),
									  chunk_from_thing(this->server_random)))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		chunk_clear(&premaster);
		return NEED_MORE;
	}
	chunk_clear(&premaster);

	this->state = STATE_KEY_EXCHANGE_RECEIVED;
	return NEED_MORE;
}

/**
 * Process Client Key Exchange
 */
static status_t process_key_exchange(private_tls_server_t *this,
									 bio_reader_t *reader)
{
	if (this->dh)
	{
		return process_key_exchange_dhe(this, reader);
	}
	return process_key_exchange_encrypted(this, reader);
}

/**
 * Process Certificate verify
 */
static status_t process_cert_verify(private_tls_server_t *this,
									bio_reader_t *reader)
{
	public_key_t *public;
	chunk_t msg;

	public = tls_find_public_key(this->peer_auth, this->peer);
	if (!public)
	{
		DBG1(DBG_TLS, "no trusted certificate found for '%Y' to verify TLS peer",
			 this->peer);
		this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
		return NEED_MORE;
	}

	msg = reader->peek(reader);
	if (!this->crypto->verify_handshake(this->crypto, public, reader))
	{
		public->destroy(public);
		DBG1(DBG_TLS, "signature verification failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECRYPT_ERROR);
		return NEED_MORE;
	}
	public->destroy(public);
	this->state = STATE_CERT_VERIFY_RECEIVED;
	this->crypto->append_handshake(this->crypto, TLS_CERTIFICATE_VERIFY, msg);
	return NEED_MORE;
}

/**
 * Process finished message
 */
static status_t process_finished(private_tls_server_t *this,
								 bio_reader_t *reader)
{
	chunk_t received, verify_data;
	u_char buf[12];

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (!reader->read_data(reader, sizeof(buf), &received))
		{
			DBG1(DBG_TLS, "received client finished too short");
			this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
			return NEED_MORE;
		}
		if (!this->crypto->calculate_finished_legacy(this->crypto,
													 "client finished", buf))
		{
			DBG1(DBG_TLS, "calculating client finished failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		verify_data = chunk_from_thing(buf);
	}
	else
	{
		received = reader->peek(reader);
		if (!this->crypto->calculate_finished(this->crypto, FALSE, &verify_data))
		{
			DBG1(DBG_TLS, "calculating client finished failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		this->crypto->change_cipher(this->crypto, TRUE);
	}

	if (!chunk_equals_const(received, verify_data))
	{
		DBG1(DBG_TLS, "received client finished invalid");
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
 * Process KeyUpdate message
 */
static status_t process_key_update(private_tls_server_t *this,
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
		DBG1(DBG_TLS, "client requested KeyUpdate");
		this->state = STATE_KEY_UPDATE_REQUESTED;
	}
	return NEED_MORE;
}

METHOD(tls_handshake_t, process, status_t,
	private_tls_server_t *this, tls_handshake_type_t type, bio_reader_t *reader)
{
	tls_handshake_type_t expected DBG_UNUSED;

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		switch (this->state)
		{
			case STATE_INIT:
				if (type == TLS_CLIENT_HELLO)
				{
					return process_client_hello(this, reader);
				}
				expected = TLS_CLIENT_HELLO;
				break;
			case STATE_HELLO_DONE:
				if (type == TLS_CERTIFICATE)
				{
					return process_certificate(this, reader);
				}
				if (this->peer)
				{
					expected = TLS_CERTIFICATE;
					break;
				}
				/* otherwise fall through to next state */
			case STATE_CERT_RECEIVED:
				if (type == TLS_CLIENT_KEY_EXCHANGE)
				{
					return process_key_exchange(this, reader);
				}
				expected = TLS_CLIENT_KEY_EXCHANGE;
				break;
			case STATE_KEY_EXCHANGE_RECEIVED:
				if (type == TLS_CERTIFICATE_VERIFY)
				{
					return process_cert_verify(this, reader);
				}
				if (this->peer)
				{
					expected = TLS_CERTIFICATE_VERIFY;
					break;
				}
				return INVALID_STATE;
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
			case STATE_INIT:
				if (type == TLS_CLIENT_HELLO)
				{
					return process_client_hello(this, reader);
				}
				expected = TLS_CLIENT_HELLO;
				break;
			case STATE_CIPHERSPEC_CHANGED_IN:
			case STATE_FINISHED_SENT:
			case STATE_FINISHED_SENT_KEY_SWITCHED:
				if (type == TLS_CERTIFICATE)
				{
					return process_certificate(this, reader);
				}
				if (this->peer)
				{
					expected = TLS_CERTIFICATE;
					break;
				}
				/* otherwise fall through to next state */
			case STATE_CERT_RECEIVED:
				if (type == TLS_CERTIFICATE_VERIFY)
				{
					return process_cert_verify(this, reader);
				}
				if (this->peer)
				{
					expected = TLS_CERTIFICATE_VERIFY;
					break;
				}
				/* otherwise fall through to next state */
			case STATE_CERT_VERIFY_RECEIVED:
				if (type == TLS_FINISHED)
				{
					return process_finished(this, reader);
				}
				return NEED_MORE;
			case STATE_FINISHED_RECEIVED:
				if (type == TLS_KEY_UPDATE)
				{
					return process_key_update(this, reader);
				}
				return INVALID_STATE;
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
 * Write public key into key share extension
 */
bool tls_write_key_share(bio_writer_t **key_share, key_exchange_t *dh)
{
	bio_writer_t *writer;
	tls_named_group_t curve;
	chunk_t pub;

	if (!dh)
	{
		return FALSE;
	}
	curve = tls_ec_group_to_curve(dh->get_method(dh));
	if (!curve || !dh->get_public_key(dh, &pub))
	{
		return FALSE;
	}
	*key_share = writer = bio_writer_create(pub.len + 7);
	writer->write_uint16(writer, curve);
	if (curve == TLS_CURVE25519 ||
		curve == TLS_CURVE448)
	{
		writer->write_data16(writer, pub);
	}
	else
	{	/* classic format (see RFC 8446, section 4.2.8.2) */
		writer->write_uint16(writer, pub.len + 1);
		writer->write_uint8(writer, TLS_ANSI_UNCOMPRESSED);
		writer->write_data(writer, pub);
	}
	free(pub.ptr);
	return TRUE;
}

/**
 * Send ServerHello message
 */
static status_t send_server_hello(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	bio_writer_t *key_share, *extensions;
	tls_version_t version;

	version = this->tls->get_version_max(this->tls);

	/* cap legacy version at TLS 1.2 for middlebox compatibility */
	writer->write_uint16(writer, min(TLS_1_2, version));

	if (this->requested_curve)
	{
		writer->write_data(writer, tls_hello_retry_request_magic);
	}
	else
	{
		writer->write_data(writer, chunk_from_thing(this->server_random));
	}

	/* session identifier if we have one */
	writer->write_data8(writer, this->session);

	/* add selected TLS cipher suite */
	writer->write_uint16(writer, this->suite);

	/* NULL compression only */
	writer->write_uint8(writer, 0);

	if (version >= TLS_1_3)
	{
		extensions = bio_writer_create(32);

		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_SUPPORTED_VERSIONS);
		extensions->write_uint16(extensions, TLS_EXT_SUPPORTED_VERSIONS);
		extensions->write_uint16(extensions, 2);
		extensions->write_uint16(extensions, version);

		DBG2(DBG_TLS, "sending extension: %N",
	   		 tls_extension_names, TLS_EXT_KEY_SHARE);
		extensions->write_uint16(extensions, TLS_EXT_KEY_SHARE);
		if (this->requested_curve)
		{
			DBG1(DBG_TLS, "requesting key exchange with %N",
				 tls_named_group_names, this->requested_curve);
			extensions->write_uint16(extensions, 2);
			extensions->write_uint16(extensions, this->requested_curve);
		}
		else
		{
			if (!tls_write_key_share(&key_share, this->dh))
			{
				this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
				extensions->destroy(extensions);
				return NEED_MORE;
			}
			extensions->write_data16(extensions, key_share->get_buf(key_share));
			key_share->destroy(key_share);
		}

		writer->write_data16(writer, extensions->get_buf(extensions));
		extensions->destroy(extensions);
	}

	*type = TLS_SERVER_HELLO;
	this->state = this->requested_curve ? STATE_INIT : STATE_HELLO_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send encrypted extensions message
 */
static status_t send_encrypted_extensions(private_tls_server_t *this,
										  tls_handshake_type_t *type,
										  bio_writer_t *writer)
{
	chunk_t shared_secret = chunk_empty;

	if (!this->dh->get_shared_secret(this->dh, &shared_secret) ||
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

	/* currently no extensions are supported */
	writer->write_uint16(writer, 0);

	*type = TLS_ENCRYPTED_EXTENSIONS;
	this->state = STATE_ENCRYPTED_EXTENSIONS_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send Certificate
 */
static status_t send_certificate(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	auth_rule_t rule;
	bio_writer_t *certs;
	chunk_t data;

	/* certificate request context as described in RFC 8446, section 4.4.2 */
	if (this->tls->get_version_max(this->tls) > TLS_1_2)
	{
		writer->write_uint8(writer, 0);
	}

	/* generate certificate payload */
	certs = bio_writer_create(256);
	cert = this->server_auth->get(this->server_auth, AUTH_RULE_SUBJECT_CERT);
	if (cert)
	{
		if (cert->get_encoding(cert, CERT_ASN1_DER, &data))
		{
			DBG1(DBG_TLS, "sending TLS server certificate '%Y'",
				 cert->get_subject(cert));
			certs->write_data24(certs, data);
			free(data.ptr);

			/* extensions see RFC 8446, section 4.4.2 */
			if (this->tls->get_version_max(this->tls) > TLS_1_2)
			{
				certs->write_uint16(certs, 0);
			}
		}
	}
	enumerator = this->server_auth->create_enumerator(this->server_auth);
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
				if (this->tls->get_version_max(this->tls) > TLS_1_2)
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
 * Send Certificate Verify
 */
static status_t send_certificate_verify(private_tls_server_t *this,
										tls_handshake_type_t *type,
										bio_writer_t *writer)
{
	if (!this->crypto->sign_handshake(this->crypto, this->private, writer,
								   	  this->hashsig))
	{
		DBG1(DBG_TLS, "signature generation failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}

	*type = TLS_CERTIFICATE_VERIFY;
	this->state = STATE_CERT_VERIFY_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/*
 * Write all available certificate authorities to output writer
 */
static void write_certificate_authorities(bio_writer_t *writer)
{
	bio_writer_t *authorities;
	enumerator_t *enumerator;
	certificate_t *cert;
	x509_t *x509;
	identification_t *id;

	authorities = bio_writer_create(64);
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr, CERT_X509,
													  KEY_RSA, NULL, TRUE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		x509 = (x509_t*)cert;
		if (x509->get_flags(x509) & X509_CA)
		{
			id = cert->get_subject(cert);
			DBG1(DBG_TLS, "sending TLS cert request for '%Y'", id);
			authorities->write_data16(authorities, id->get_encoding(id));
		}
	}
	enumerator->destroy(enumerator);
	writer->write_data16(writer, authorities->get_buf(authorities));
	authorities->destroy(authorities);
}

/**
 * Send Certificate Request
 */
static status_t send_certificate_request(private_tls_server_t *this,
										 tls_handshake_type_t *type,
										 bio_writer_t *writer)
{
	bio_writer_t *authorities, *supported, *extensions;

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		supported = bio_writer_create(4);
		/* we propose both RSA and ECDSA */
		supported->write_uint8(supported, TLS_RSA_SIGN);
		supported->write_uint8(supported, TLS_ECDSA_SIGN);
		writer->write_data8(writer, supported->get_buf(supported));
		supported->destroy(supported);
		if (this->tls->get_version_max(this->tls) >= TLS_1_2)
		{
			this->crypto->get_signature_algorithms(this->crypto, writer, TRUE);
		}

		if (this->send_certreq_authorities)
		{
			write_certificate_authorities(writer);
		}
		else
		{
			writer->write_data16(writer, chunk_empty);
		}
	}
	else
	{
		/* certificate request context as described in RFC 8446, section 4.3.2 */
		writer->write_uint8(writer, 0);

		extensions = bio_writer_create(32);

		if (this->send_certreq_authorities)
		{
			DBG2(DBG_TLS, "sending extension: %N",
				 tls_extension_names, TLS_EXT_CERTIFICATE_AUTHORITIES);
			authorities = bio_writer_create(64);
			write_certificate_authorities(authorities);
			extensions->write_uint16(extensions, TLS_EXT_CERTIFICATE_AUTHORITIES);
			extensions->write_data16(extensions, authorities->get_buf(authorities));
			authorities->destroy(authorities);
		}

		DBG2(DBG_TLS, "sending extension: %N",
			 tls_extension_names, TLS_EXT_SIGNATURE_ALGORITHMS);
		extensions->write_uint16(extensions, TLS_EXT_SIGNATURE_ALGORITHMS);
		supported = bio_writer_create(32);
		this->crypto->get_signature_algorithms(this->crypto, supported, TRUE);
		extensions->write_data16(extensions, supported->get_buf(supported));
		supported->destroy(supported);
		writer->write_data16(writer, extensions->get_buf(extensions));
		extensions->destroy(extensions);
	}

	*type = TLS_CERTIFICATE_REQUEST;
	this->state = STATE_CERTREQ_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Try to find a curve/group supported by both, client and server
 */
static bool find_supported_curve(private_tls_server_t *this,
								 tls_named_group_t *curve,
								 key_exchange_method_t *group)
{
	tls_named_group_t current;
	key_exchange_method_t current_group;
	enumerator_t *enumerator;

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, &current_group, &current))
	{
		if (peer_supports_curve(this, current))
		{
			*curve = current;
			*group = current_group;
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

/**
 * Send Server key Exchange
 */
static status_t send_server_key_exchange(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer,
							key_exchange_method_t group)
{
	diffie_hellman_params_t *params = NULL;
	tls_named_group_t curve;
	chunk_t chunk;

	if (key_exchange_is_ecdh(group))
	{
		curve = tls_ec_group_to_curve(group);
		if (!curve || (!peer_supports_curve(this, curve) &&
					   !find_supported_curve(this, &curve, &group)))
		{
			DBG1(DBG_TLS, "no EC group supported by client and server");
			this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
			return NEED_MORE;
		}
		DBG2(DBG_TLS, "selected ECDH group %N", tls_named_group_names, curve);
		writer->write_uint8(writer, TLS_ECC_NAMED_CURVE);
		writer->write_uint16(writer, curve);
	}
	else
	{
		params = diffie_hellman_get_params(group);
		if (!params)
		{
			DBG1(DBG_TLS, "no parameters found for DH group %N",
				 key_exchange_method_names, group);
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		DBG2(DBG_TLS, "selected DH group %N", key_exchange_method_names, group);
		writer->write_data16(writer, params->prime);
		writer->write_data16(writer, params->generator);
	}
	this->dh = lib->crypto->create_ke(lib->crypto, group);
	if (!this->dh)
	{
		DBG1(DBG_TLS, "DH group %N not supported",
			 key_exchange_method_names, group);
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (!this->dh->get_public_key(this->dh, &chunk))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (params)
	{
		writer->write_data16(writer, chunk);
	}
	else if (group != CURVE_25519 &&
			 group != CURVE_448)
	{	/* ECP uses 8bit length header only, but a point format */
		writer->write_uint8(writer, chunk.len + 1);
		writer->write_uint8(writer, TLS_ANSI_UNCOMPRESSED);
		writer->write_data(writer, chunk);
	}
	else
	{	/* ECPoint uses an 8-bit length header only */
		writer->write_data8(writer, chunk);
	}
	free(chunk.ptr);

	chunk = chunk_cat("ccc", chunk_from_thing(this->client_random),
				chunk_from_thing(this->server_random), writer->get_buf(writer));
	if (!this->private || !this->crypto->sign(this->crypto, this->private,
											  writer, chunk, this->hashsig))
	{
		DBG1(DBG_TLS, "signing DH parameters failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		free(chunk.ptr);
		return NEED_MORE;
	}
	free(chunk.ptr);
	*type = TLS_SERVER_KEY_EXCHANGE;
	this->state = STATE_KEY_EXCHANGE_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send Hello Done
 */
static status_t send_hello_done(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	*type = TLS_SERVER_HELLO_DONE;
	this->state = STATE_HELLO_DONE;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Send Finished
 */
static status_t send_finished(private_tls_server_t *this,
							  tls_handshake_type_t *type, bio_writer_t *writer)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		char buf[12];

		if (!this->crypto->calculate_finished_legacy(this->crypto,
													 "server finished", buf))
		{
			DBG1(DBG_TLS, "calculating server finished data failed");
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return FAILED;
		}

		writer->write_data(writer, chunk_from_thing(buf));
	}
	else
	{
		chunk_t verify_data;

		if (!this->crypto->calculate_finished(this->crypto, TRUE, &verify_data))
		{
			DBG1(DBG_TLS, "calculating server finished data failed");
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
static status_t send_key_update(private_tls_server_t *this,
								tls_handshake_type_t *type, bio_writer_t *writer)
{
	*type = TLS_KEY_UPDATE;

	/* we currently only send this as reply, so we never request an update */
	writer->write_uint8(writer, 0);

	this->state = STATE_KEY_UPDATE_SENT;
	return NEED_MORE;
}

METHOD(tls_handshake_t, build, status_t,
	private_tls_server_t *this, tls_handshake_type_t *type, bio_writer_t *writer)
{
	key_exchange_method_t group;

	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		switch (this->state)
		{
			case STATE_HELLO_RECEIVED:
				return send_server_hello(this, type, writer);
			case STATE_HELLO_SENT:
				return send_certificate(this, type, writer);
			case STATE_CERT_SENT:
				group = this->crypto->get_dh_group(this->crypto);
				if (group)
				{
					return send_server_key_exchange(this, type, writer, group);
				}
				/* otherwise fall through to next state */
			case STATE_KEY_EXCHANGE_SENT:
				if (this->peer)
				{
					return send_certificate_request(this, type, writer);
				}
				/* otherwise fall through to next state */
			case STATE_CERTREQ_SENT:
				return send_hello_done(this, type, writer);
			case STATE_CIPHERSPEC_CHANGED_OUT:
				return send_finished(this, type, writer);
			case STATE_FINISHED_SENT:
				return INVALID_STATE;
			default:
				return INVALID_STATE;
		}
	}
	else
	{
		switch (this->state)
		{
			case STATE_HELLO_RECEIVED:
				return send_server_hello(this, type, writer);
			case STATE_HELLO_SENT:
			case STATE_CIPHERSPEC_CHANGED_OUT:
				return send_encrypted_extensions(this, type, writer);
			case STATE_ENCRYPTED_EXTENSIONS_SENT:
				if (this->peer)
				{
					return send_certificate_request(this, type, writer);
				}
				/* otherwise fall through to next state */
			case STATE_CERTREQ_SENT:
				return send_certificate(this, type, writer);
			case STATE_CERT_SENT:
				return send_certificate_verify(this, type, writer);
			case STATE_CERT_VERIFY_SENT:
				return send_finished(this, type, writer);
			case STATE_FINISHED_SENT:
				if (!this->crypto->derive_app_keys(this->crypto))
				{
					this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
					return NEED_MORE;
				}
				/* inbound key switches after process client finished message */
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
				this->state = STATE_FINISHED_RECEIVED;
			default:
				return INVALID_STATE;
		}
	}
}

METHOD(tls_handshake_t, cipherspec_changed, bool,
	private_tls_server_t *this, bool inbound)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (inbound)
		{
			if (this->resume)
			{
				return this->state == STATE_FINISHED_SENT;
			}
			if (this->peer)
			{
				return this->state == STATE_CERT_VERIFY_RECEIVED;
			}
			return this->state == STATE_KEY_EXCHANGE_RECEIVED;
		}
		else
		{
			if (this->resume)
			{
				return this->state == STATE_HELLO_SENT;
			}
			return this->state == STATE_FINISHED_RECEIVED;
		}
		return FALSE;
	}
	else
	{
		if (inbound)
		{	/* accept ChangeCipherSpec after ServerFinish or HelloRetryRequest */
			return this->state == STATE_FINISHED_SENT ||
				   this->state == STATE_FINISHED_SENT_KEY_SWITCHED ||
				   retrying(this);
		}
		else
		{
			return this->state == STATE_HELLO_SENT;
		}
	}
}

METHOD(tls_handshake_t, change_cipherspec, void,
	private_tls_server_t *this, bool inbound)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		this->crypto->change_cipher(this->crypto, inbound);
	}

	if (retrying(this))
	{	/* client might send a ChangeCipherSpec after a HelloRetryRequest and
		 * before a new ClientHello which should not cause any state changes */
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
	private_tls_server_t *this)
{
	if (this->tls->get_version_max(this->tls) < TLS_1_3)
	{
		if (this->resume)
		{
			return this->state == STATE_FINISHED_RECEIVED;
		}
		return this->state == STATE_FINISHED_SENT;
	}
	else
	{
		return this->state == STATE_FINISHED_RECEIVED;
	}
}

METHOD(tls_handshake_t, get_peer_id, identification_t*,
	private_tls_server_t *this)
{
	return this->peer;
}

METHOD(tls_handshake_t, get_server_id, identification_t*,
	private_tls_server_t *this)
{
	return this->server;
}

METHOD(tls_handshake_t, get_auth, auth_cfg_t*,
	private_tls_server_t *this)
{
	return this->peer_auth;
}

METHOD(tls_handshake_t, destroy, void,
	private_tls_server_t *this)
{
	DESTROY_IF(this->private);
	DESTROY_IF(this->dh);
	DESTROY_IF(this->peer);
	this->server->destroy(this->server);
	this->peer_auth->destroy(this->peer_auth);
	this->server_auth->destroy(this->server_auth);
	free(this->hashsig.ptr);
	free(this->curves.ptr);
	free(this->session.ptr);
	free(this);
}

/**
 * See header
 */
tls_server_t *tls_server_create(tls_t *tls,
						tls_crypto_t *crypto, tls_alert_t *alert,
						identification_t *server, identification_t *peer)
{
	private_tls_server_t *this;

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
		.tls = tls,
		.crypto = crypto,
		.alert = alert,
		.server = server->clone(server),
		.peer = peer ? peer->clone(peer) : NULL,
		.state = STATE_INIT,
		.peer_auth = auth_cfg_create(),
		.server_auth = auth_cfg_create(),
		.send_certreq_authorities = lib->settings->get_bool(lib->settings,
											"%s.tls.send_certreq_authorities",
											TRUE, lib->ns),
	);

	return &this->public;
}

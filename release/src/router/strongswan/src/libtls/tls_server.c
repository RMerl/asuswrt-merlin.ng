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

#include "tls_server.h"

#include <time.h>

#include <utils/debug.h>
#include <credentials/certificates/x509.h>

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
	 * Is it acceptable if we couldn't verify the peer certificate?
	 */
	bool peer_auth_optional;

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
	diffie_hellman_t *dh;

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
};

/**
 * Find a cipher suite and a server key
 */
static bool select_suite_and_key(private_tls_server_t *this,
								 tls_cipher_suite_t *suites, int count)
{
	private_key_t *key;
	key_type_t type;

	key = lib->credmgr->get_private(lib->credmgr, KEY_ANY, this->server,
									this->server_auth);
	if (!key)
	{
		DBG1(DBG_TLS, "no usable TLS server certificate found for '%Y'",
			 this->server);
		return FALSE;
	}
	this->suite = this->crypto->select_cipher_suite(this->crypto,
											suites, count, key->get_type(key));
	if (!this->suite)
	{	/* no match for this key, try to find another type */
		if (key->get_type(key) == KEY_ECDSA)
		{
			type = KEY_RSA;
		}
		else
		{
			type = KEY_ECDSA;
		}
		key->destroy(key);

		this->suite = this->crypto->select_cipher_suite(this->crypto,
											suites, count, type);
		if (!this->suite)
		{
			DBG1(DBG_TLS, "received cipher suites unacceptable");
			return FALSE;
		}
		this->server_auth->destroy(this->server_auth);
		this->server_auth = auth_cfg_create();
		key = lib->credmgr->get_private(lib->credmgr, type, this->server,
										this->server_auth);
		if (!key)
		{
			DBG1(DBG_TLS, "received cipher suites unacceptable");
			return FALSE;
		}
	}
	this->private = key;
	return TRUE;
}

/**
 * Process client hello message
 */
static status_t process_client_hello(private_tls_server_t *this,
									 bio_reader_t *reader)
{
	uint16_t version, extension;
	chunk_t random, session, ciphers, compression, ext = chunk_empty;
	bio_reader_t *extensions;
	tls_cipher_suite_t *suites;
	int count, i;
	rng_t *rng;

	this->crypto->append_handshake(this->crypto,
								   TLS_CLIENT_HELLO, reader->peek(reader));

	if (!reader->read_uint16(reader, &version) ||
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

	if (ext.len)
	{
		extensions = bio_reader_create(ext);
		while (extensions->remaining(extensions))
		{
			if (!extensions->read_uint16(extensions, &extension) ||
				!extensions->read_data16(extensions, &ext))
			{
				DBG1(DBG_TLS, "received invalid ClientHello Extensions");
				this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
				extensions->destroy(extensions);
				return NEED_MORE;
			}
			DBG2(DBG_TLS, "received TLS '%N' extension",
				 tls_extension_names, extension);
			DBG3(DBG_TLS, "%B", &ext);
			switch (extension)
			{
				case TLS_EXT_SIGNATURE_ALGORITHMS:
					this->hashsig = chunk_clone(ext);
					break;
				case TLS_EXT_ELLIPTIC_CURVES:
					this->curves_received = TRUE;
					this->curves = chunk_clone(ext);
					break;
				default:
					break;
			}
		}
		extensions->destroy(extensions);
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

	if (!this->tls->set_version(this->tls, version))
	{
		DBG1(DBG_TLS, "negotiated version %N not supported",
			 tls_version_names, version);
		this->alert->add(this->alert, TLS_FATAL, TLS_PROTOCOL_VERSION);
		return NEED_MORE;
	}

	this->client_version = version;
	this->suite = this->crypto->resume_session(this->crypto, session, this->peer,
										chunk_from_thing(this->client_random),
										chunk_from_thing(this->server_random));
	if (this->suite)
	{
		this->session = chunk_clone(session);
		this->resume = TRUE;
		DBG1(DBG_TLS, "resumed %N using suite %N",
			 tls_version_names, this->tls->get_version(this->tls),
			 tls_cipher_suite_names, this->suite);
	}
	else
	{
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
		rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
		if (!rng || !rng->allocate_bytes(rng, SESSION_ID_SIZE, &this->session))
		{
			DBG1(DBG_TLS, "generating TLS session identifier failed, skipped");
		}
		DESTROY_IF(rng);
		DBG1(DBG_TLS, "negotiated %N using suite %N",
			 tls_version_names, this->tls->get_version(this->tls),
			 tls_cipher_suite_names, this->suite);
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
				this->peer_auth->add(this->peer_auth,
									 AUTH_HELPER_SUBJECT_CERT, cert);
				DBG1(DBG_TLS, "received TLS peer certificate '%Y'",
					 cert->get_subject(cert));
				first = FALSE;
				if (this->peer == NULL)
				{	/* apply identity to authenticate */
					this->peer = cert->get_subject(cert);
					this->peer = this->peer->clone(this->peer);
					this->peer_auth_optional = TRUE;
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
		this->private->decrypt(this->private,
							   ENCRYPT_RSA_PKCS1, encrypted, &decrypted))
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
	bool ec;

	this->crypto->append_handshake(this->crypto,
								   TLS_CLIENT_KEY_EXCHANGE, reader->peek(reader));

	ec = diffie_hellman_group_is_ec(this->dh->get_dh_group(this->dh));
	if ((ec && !reader->read_data8(reader, &pub)) ||
		(!ec && (!reader->read_data16(reader, &pub) || pub.len == 0)))
	{
		DBG1(DBG_TLS, "received invalid Client Key Exchange");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}

	if (ec)
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
	if (!this->dh->set_other_public_value(this->dh, pub))
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
	bool verified = FALSE;
	enumerator_t *enumerator;
	public_key_t *public;
	auth_cfg_t *auth;
	bio_reader_t *sig;

	enumerator = lib->credmgr->create_public_enumerator(lib->credmgr,
									KEY_ANY, this->peer, this->peer_auth, TRUE);
	while (enumerator->enumerate(enumerator, &public, &auth))
	{
		sig = bio_reader_create(reader->peek(reader));
		verified = this->crypto->verify_handshake(this->crypto, public, sig);
		sig->destroy(sig);
		if (verified)
		{
			this->peer_auth->merge(this->peer_auth, auth, FALSE);
			break;
		}
		DBG1(DBG_TLS, "signature verification failed, trying another key");
	}
	enumerator->destroy(enumerator);

	if (!verified)
	{
		DBG1(DBG_TLS, "no trusted certificate found for '%Y' to verify TLS peer",
			 this->peer);
		if (!this->peer_auth_optional)
		{	/* client authentication is required */
			this->alert->add(this->alert, TLS_FATAL, TLS_CERTIFICATE_UNKNOWN);
			return NEED_MORE;
		}
		/* reset peer identity, we couldn't authenticate it */
		this->peer->destroy(this->peer);
		this->peer = NULL;
		this->state = STATE_KEY_EXCHANGE_RECEIVED;
	}
	else
	{
		this->state = STATE_CERT_VERIFY_RECEIVED;
	}
	this->crypto->append_handshake(this->crypto,
								   TLS_CERTIFICATE_VERIFY, reader->peek(reader));
	return NEED_MORE;
}

/**
 * Process finished message
 */
static status_t process_finished(private_tls_server_t *this,
								 bio_reader_t *reader)
{
	chunk_t received;
	char buf[12];

	if (!reader->read_data(reader, sizeof(buf), &received))
	{
		DBG1(DBG_TLS, "received client finished too short");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECODE_ERROR);
		return NEED_MORE;
	}
	if (!this->crypto->calculate_finished(this->crypto, "client finished", buf))
	{
		DBG1(DBG_TLS, "calculating client finished failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (!chunk_equals_const(received, chunk_from_thing(buf)))
	{
		DBG1(DBG_TLS, "received client finished invalid");
		this->alert->add(this->alert, TLS_FATAL, TLS_DECRYPT_ERROR);
		return NEED_MORE;
	}

	this->crypto->append_handshake(this->crypto, TLS_FINISHED, received);
	this->state = STATE_FINISHED_RECEIVED;
	return NEED_MORE;
}

METHOD(tls_handshake_t, process, status_t,
	private_tls_server_t *this, tls_handshake_type_t type, bio_reader_t *reader)
{
	tls_handshake_type_t expected;

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
	DBG1(DBG_TLS, "TLS %N expected, but received %N",
		 tls_handshake_type_names, expected, tls_handshake_type_names, type);
	this->alert->add(this->alert, TLS_FATAL, TLS_UNEXPECTED_MESSAGE);
	return NEED_MORE;
}

/**
 * Send ServerHello message
 */
static status_t send_server_hello(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	/* TLS version */
	writer->write_uint16(writer, this->tls->get_version(this->tls));
	writer->write_data(writer, chunk_from_thing(this->server_random));

	/* session identifier if we have one */
	writer->write_data8(writer, this->session);

	/* add selected TLS cipher suite */
	writer->write_uint16(writer, this->suite);

	/* NULL compression only */
	writer->write_uint8(writer, 0);

	*type = TLS_SERVER_HELLO;
	this->state = STATE_HELLO_SENT;
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
 * Send Certificate Request
 */
static status_t send_certificate_request(private_tls_server_t *this,
							tls_handshake_type_t *type, bio_writer_t *writer)
{
	bio_writer_t *authorities, *supported;
	enumerator_t *enumerator;
	certificate_t *cert;
	x509_t *x509;
	identification_t *id;

	supported = bio_writer_create(4);
	/* we propose both RSA and ECDSA */
	supported->write_uint8(supported, TLS_RSA_SIGN);
	supported->write_uint8(supported, TLS_ECDSA_SIGN);
	writer->write_data8(writer, supported->get_buf(supported));
	supported->destroy(supported);
	if (this->tls->get_version(this->tls) >= TLS_1_2)
	{
		this->crypto->get_signature_algorithms(this->crypto, writer);
	}

	authorities = bio_writer_create(64);
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
												CERT_X509, KEY_RSA, NULL, TRUE);
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

	*type = TLS_CERTIFICATE_REQUEST;
	this->state = STATE_CERTREQ_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));
	return NEED_MORE;
}

/**
 * Get the TLS curve of a given EC DH group
 */
static tls_named_curve_t ec_group_to_curve(private_tls_server_t *this,
										   diffie_hellman_group_t group)
{
	diffie_hellman_group_t current;
	tls_named_curve_t curve;
	enumerator_t *enumerator;

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, &current, &curve))
	{
		if (current == group)
		{
			enumerator->destroy(enumerator);
			return curve;
		}
	}
	enumerator->destroy(enumerator);
	return 0;
}

/**
 * Check if the peer supports a given TLS curve
 */
bool peer_supports_curve(private_tls_server_t *this, tls_named_curve_t curve)
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
 * Try to find a curve supported by both, client and server
 */
static bool find_supported_curve(private_tls_server_t *this,
								 tls_named_curve_t *curve)
{
	tls_named_curve_t current;
	enumerator_t *enumerator;

	enumerator = this->crypto->create_ec_enumerator(this->crypto);
	while (enumerator->enumerate(enumerator, NULL, &current))
	{
		if (peer_supports_curve(this, current))
		{
			*curve = current;
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
							diffie_hellman_group_t group)
{
	diffie_hellman_params_t *params = NULL;
	tls_named_curve_t curve;
	chunk_t chunk;

	if (diffie_hellman_group_is_ec(group))
	{
		curve = ec_group_to_curve(this, group);
		if (!curve || (!peer_supports_curve(this, curve) &&
					   !find_supported_curve(this, &curve)))
		{
			DBG1(DBG_TLS, "no EC group supported by client and server");
			this->alert->add(this->alert, TLS_FATAL, TLS_HANDSHAKE_FAILURE);
			return NEED_MORE;
		}
		DBG2(DBG_TLS, "selected ECDH group %N", tls_named_curve_names, curve);
		writer->write_uint8(writer, TLS_ECC_NAMED_CURVE);
		writer->write_uint16(writer, curve);
	}
	else
	{
		params = diffie_hellman_get_params(group);
		if (!params)
		{
			DBG1(DBG_TLS, "no parameters found for DH group %N",
				 diffie_hellman_group_names, group);
			this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
			return NEED_MORE;
		}
		DBG2(DBG_TLS, "selected DH group %N", diffie_hellman_group_names, group);
		writer->write_data16(writer, params->prime);
		writer->write_data16(writer, params->generator);
	}
	this->dh = lib->crypto->create_dh(lib->crypto, group);
	if (!this->dh)
	{
		DBG1(DBG_TLS, "DH group %N not supported",
			 diffie_hellman_group_names, group);
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (!this->dh->get_my_public_value(this->dh, &chunk))
	{
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return NEED_MORE;
	}
	if (params)
	{
		writer->write_data16(writer, chunk);
	}
	else
	{	/* ECP uses 8bit length header only, but a point format */
		writer->write_uint8(writer, chunk.len + 1);
		writer->write_uint8(writer, TLS_ANSI_UNCOMPRESSED);
		writer->write_data(writer, chunk);
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
	char buf[12];

	if (!this->crypto->calculate_finished(this->crypto, "server finished", buf))
	{
		DBG1(DBG_TLS, "calculating server finished data failed");
		this->alert->add(this->alert, TLS_FATAL, TLS_INTERNAL_ERROR);
		return FAILED;
	}

	writer->write_data(writer, chunk_from_thing(buf));

	*type = TLS_FINISHED;
	this->state = STATE_FINISHED_SENT;
	this->crypto->append_handshake(this->crypto, *type, writer->get_buf(writer));

	return NEED_MORE;
}

METHOD(tls_handshake_t, build, status_t,
	private_tls_server_t *this, tls_handshake_type_t *type, bio_writer_t *writer)
{
	diffie_hellman_group_t group;

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
			return send_certificate_request(this, type, writer);
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

METHOD(tls_handshake_t, cipherspec_changed, bool,
	private_tls_server_t *this, bool inbound)
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

METHOD(tls_handshake_t, change_cipherspec, void,
	private_tls_server_t *this, bool inbound)
{
	this->crypto->change_cipher(this->crypto, inbound);
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
	if (this->resume)
	{
		return this->state == STATE_FINISHED_RECEIVED;
	}
	return this->state == STATE_FINISHED_SENT;
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
	);

	return &this->public;
}

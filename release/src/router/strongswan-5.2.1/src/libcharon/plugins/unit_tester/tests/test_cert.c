/*
 * Copyright (C) 2008 Martin Willi
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

#include <library.h>
#include <daemon.h>
#include <credentials/certificates/x509.h>

/*******************************************************************************
 * X509 certificate generation and parsing
 ******************************************************************************/
bool test_cert_x509()
{
	private_key_t *ca_key, *peer_key;
	public_key_t *public;
	certificate_t *ca_cert, *peer_cert, *parsed;
	identification_t *issuer, *subject;
	u_int32_t serial = htonl(0);
	chunk_t encoding;

	issuer = identification_create_from_string("CN=CA, OU=Test, O=strongSwan");
	subject = identification_create_from_string("CN=Peer, OU=Test, O=strongSwan");

	ca_key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
						BUILD_KEY_SIZE, 1024, BUILD_END);
	peer_key = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
						BUILD_KEY_SIZE, 1024, BUILD_END);
	if (!ca_key)
	{
		return FALSE;
	}
	ca_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_SIGNING_KEY, ca_key,
						BUILD_SUBJECT, issuer,
						BUILD_SERIAL, chunk_from_thing(serial),
						BUILD_X509_FLAG, X509_CA,
						BUILD_END);
	if (!ca_cert)
	{
		return FALSE;
	}

	ca_cert->get_encoding(ca_cert, CERT_ASN1_DER, &encoding);
	parsed = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_BLOB_ASN1_DER, encoding,
						BUILD_END);
	chunk_free(&encoding);
	if (!parsed)
	{
		return FALSE;
	}
	if (!parsed->issued_by(parsed, ca_cert, NULL))
	{
		return FALSE;
	}
	parsed->destroy(parsed);

	serial = htonl(ntohl(serial) + 1);
	public = peer_key->get_public_key(peer_key);
	peer_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_SIGNING_KEY, ca_key,
						BUILD_SIGNING_CERT, ca_cert,
						BUILD_PUBLIC_KEY, public,
						BUILD_SUBJECT, subject,
						BUILD_SERIAL, chunk_from_thing(serial),
						BUILD_END);
	public->destroy(public);
	if (!peer_cert)
	{
		return FALSE;
	}

	peer_cert->get_encoding(peer_cert, CERT_ASN1_DER, &encoding);
	parsed = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
						BUILD_BLOB_ASN1_DER, encoding,
						BUILD_END);
	chunk_free(&encoding);
	if (!parsed)
	{
		return FALSE;
	}
	if (!parsed->issued_by(parsed, ca_cert, NULL))
	{
		return FALSE;
	}
	parsed->destroy(parsed);

	ca_cert->destroy(ca_cert);
	ca_key->destroy(ca_key);
	peer_cert->destroy(peer_cert);
	peer_key->destroy(peer_key);
	issuer->destroy(issuer);
	subject->destroy(subject);
	return TRUE;
}



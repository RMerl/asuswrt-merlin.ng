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

/**
 * @defgroup credential_set credential_set
 * @{ @ingroup credentials
 */

#ifndef CREDENTIAL_SET_H_
#define CREDENTIAL_SET_H_

typedef struct credential_set_t credential_set_t;

#include <credentials/keys/public_key.h>
#include <credentials/keys/shared_key.h>
#include <credentials/certificates/certificate.h>

/**
 * A set of credentials.
 *
 * Contains private keys, shared keys and different kinds of certificates.
 * Enumerators are used because queries might return multiple matches.
 * Filter parameters restrict enumeration over specific items only.
 * See credential_manager_t for an overview of the credential framework.
 *
 * A credential set enumerator may not block the credential set, i.e. multiple
 * threads must be able to hold multiple enumerators, as the credential manager
 * is higly parallelized. The best way to achieve this is by using shared
 * read locks for the enumerators only. Otherwise deadlocks will occur.
 * The writing cache_cert() routine is called by the manager only if no
 * enumerator is alive, so it is save to use a write lock there.
 */
struct credential_set_t {

	/**
	 * Create an enumerator over private keys (private_key_t).
	 *
	 * The id is either a key identifier of the requested key, or an identity
	 * of the key owner.
	 *
	 * @param type		type of requested private key
	 * @param id		key identifier/owner
	 * @return			enumerator over private_key_t's.
	 */
	enumerator_t *(*create_private_enumerator)(credential_set_t *this,
						key_type_t type, identification_t *id);
	/**
	 * Create an enumerator over certificates (certificate_t).
	 *
	 * @param cert		kind of certificate
	 * @param key		kind of key in certificate
	 * @param id		identity (subject) this certificate belongs to
	 * @param trusted	whether the certificate must be trustworthy
	 * @return			enumerator as described above
	 */
	enumerator_t *(*create_cert_enumerator)(credential_set_t *this,
						certificate_type_t cert, key_type_t key,
						identification_t *id, bool trusted);
	/**
	 * Create an enumerator over shared keys (shared_key_t).
	 *
	 * The enumerator enumerates over:
	 *  shared_key_t*, id_match_t me, id_match_t other
	 * But must accept NULL values for the id_matches.
	 *
	 * @param type		kind of requested shared key
	 * @param me		own identity
	 * @param other		other identity who owns that secret
	 * @return			enumerator as described above
	 */
	enumerator_t *(*create_shared_enumerator)(credential_set_t *this,
						shared_key_type_t type,
						identification_t *me, identification_t *other);

	/**
	 * Create an enumerator over certificate distribution points.
	 *
	 * @param type		type of the certificate to get a CDP
	 * @param id		identification of the distributed certificate
	 * @return			an enumerator over CDPs as char*
	 */
	enumerator_t *(*create_cdp_enumerator)(credential_set_t *this,
						certificate_type_t type, identification_t *id);

	/**
	 * Cache a certificate in the credential set.
	 *
	 * The caching policy is implementation dependent. The sets may cache the
	 * certificate in-memory, persistent on disk or not at all.
	 *
	 * @param cert		certificate to cache
	 */
	void (*cache_cert)(credential_set_t *this, certificate_t *cert);
};

#endif /** CREDENTIAL_SET_H_ @}*/

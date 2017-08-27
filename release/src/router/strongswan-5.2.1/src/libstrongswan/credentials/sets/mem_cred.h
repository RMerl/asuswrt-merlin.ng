/*
 * Copyright (C) 2010-2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

/**
 * @defgroup mem_cred mem_cred
 * @{ @ingroup sets
 */

#ifndef MEM_CRED_H_
#define MEM_CRED_H_

typedef struct mem_cred_t mem_cred_t;

#include <credentials/credential_set.h>
#include <credentials/certificates/crl.h>
#include <collections/linked_list.h>

/**
 * Generic in-memory credential set.
 */
struct mem_cred_t {

	/**
	 * Implements credential_set_t.
	 */
	credential_set_t set;

	/**
	 * Add a certificate to the credential set.
	 *
	 * @param trusted		TRUE to serve certificate as trusted
	 * @param cert			certificate, reference gets owned by set
	 */
	void (*add_cert)(mem_cred_t *this, bool trusted, certificate_t *cert);

	/**
	 * Add a certificate to the credential set, returning a reference to it or
	 * to a cached duplicate.
	 *
	 * @param trusted		TRUE to serve certificate as trusted
	 * @param cert			certificate, reference gets owned by set
	 * @return				reference to cert or a previously cached duplicate
	 */
	certificate_t *(*add_cert_ref)(mem_cred_t *this, bool trusted,
								   certificate_t *cert);

	/**
	 * Add an X.509 CRL to the credential set.
	 *
	 * @param crl			CRL, gets owned by set
	 * @return				TRUE, if the CRL is newer than an existing one (or
	 *						new at all)
	 */
	bool (*add_crl)(mem_cred_t *this, crl_t *crl);

	/**
	 * Add a private key to the credential set.
	 *
	 * @param key			key, reference gets owned by set
	 */
	void (*add_key)(mem_cred_t *this, private_key_t *key);

	/**
	 * Add a shared key to the credential set.
	 *
	 * @param shared		shared key to add, gets owned by set
	 * @param ...			NULL terminated list of owners (identification_t*)
	 */
	void (*add_shared)(mem_cred_t *this, shared_key_t *shared, ...);

	/**
	 * Add a shared key to the credential set.
	 *
	 * @param shared		shared key to add, gets owned by set
	 * @param owners		list of owners (identification_t*), gets owned
	 */
	void (*add_shared_list)(mem_cred_t *this, shared_key_t *shared,
							linked_list_t *owners);
	/**
	 * Add a certificate distribution point to the set.
	 *
	 * @param type			type of the certificate
	 * @param id			certificate ID CDP has a cert for, gets cloned
	 * @param uri			CDP URI, gets strduped
	 */
	void (*add_cdp)(mem_cred_t *this, certificate_type_t type,
					identification_t *id, char *uri);

	/**
	 * Replace all secrets (private and shared keys) in this credential set
	 * with those of another.
	 *
	 * @param other			credential set to get secrets from
	 * @param clone			TRUE to clone secrets, FALSE to adopt them (they
	 *						get removed from the other set)
	 */
	void (*replace_secrets)(mem_cred_t *this, mem_cred_t *other, bool clone);

	/**
	 * Clear all credentials from the credential set.
	 */
	void (*clear)(mem_cred_t *this);

	/**
	 * Clear the secrets (private and shared keys, not the certificates) from
	 * the credential set.
	 */
	void (*clear_secrets)(mem_cred_t *this);

	/**
	 * Destroy a mem_cred_t.
	 */
	void (*destroy)(mem_cred_t *this);
};

/**
 * Create a mem_cred instance.
 */
mem_cred_t *mem_cred_create();

#endif /** MEM_CRED_H_ @}*/

/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

/**
 * @defgroup stroke_cred stroke_cred
 * @{ @ingroup stroke
 */

#ifndef STROKE_CRED_H_
#define STROKE_CRED_H_

#include <stdio.h>

#include <stroke_msg.h>
#include <credentials/credential_set.h>
#include <credentials/certificates/certificate.h>
#include <collections/linked_list.h>

#include "stroke_ca.h"

typedef struct stroke_cred_t stroke_cred_t;

/**
 * Stroke in-memory credential storage.
 */
struct stroke_cred_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Reread secrets from config files.
	 *
	 * @param msg		stroke message
	 * @param prompt	I/O channel to prompt for private key passhprase
	 */
	void (*reread)(stroke_cred_t *this, stroke_msg_t *msg, FILE *prompt);

	/**
	 * Load a peer certificate and serve it through the credential_set.
	 *
	 * @param filename		file to load peer cert from
	 * @return				reference to loaded certificate, or NULL
	 */
	certificate_t* (*load_peer)(stroke_cred_t *this, char *filename);

	/**
	 * Load a raw public key and serve it through the credential_set.
	 *
	 * @param filename		encoding or file to load raw public key from
	 * @param identity		identity of the raw public key owner
	 * @return				reference to loaded raw public key, or NULL
	 */
	certificate_t* (*load_pubkey)(stroke_cred_t *this, char *filename,
								  identification_t *identity);

	/**
	 * Add a shared secret to serve through the credential_set.
	 *
	 * @param shared		shared key to add, gets owned
	 * @param owners		list of owners (identification_t*), gets owned
	 */
	void (*add_shared)(stroke_cred_t *this, shared_key_t *shared,
					   linked_list_t *owners);

	/**
	 * Enable/Disable CRL caching to disk.
	 *
	 * @param enabled		TRUE to enable, FALSE to disable
	 */
	void (*cachecrl)(stroke_cred_t *this, bool enabled);

	/**
	 * Destroy a stroke_cred instance.
	 */
	void (*destroy)(stroke_cred_t *this);
};

/**
 * Create a stroke_cred instance.
 */
stroke_cred_t *stroke_cred_create(stroke_ca_t *ca);

#endif /** STROKE_CRED_H_ @}*/

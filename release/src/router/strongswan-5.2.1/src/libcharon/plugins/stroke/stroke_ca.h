/*
 * Copyright (C) 2008 Tobias Brunner
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

/**
 * @defgroup stroke_ca stroke_ca
 * @{ @ingroup stroke
 */

#ifndef STROKE_CA_H_
#define STROKE_CA_H_

#include <stroke_msg.h>

#include "stroke_cred.h"

typedef struct stroke_ca_t stroke_ca_t;

/**
 * ipsec.conf ca section handling.
 */
struct stroke_ca_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Add a CA to the set using a stroke_msg_t.
	 *
	 * @param msg		stroke message containing CA info
	 */
	void (*add)(stroke_ca_t *this, stroke_msg_t *msg);

	/**
	 * Remove a CA from the set using a stroke_msg_t.
	 *
	 * @param msg		stroke message containing CA info
	 */
	void (*del)(stroke_ca_t *this, stroke_msg_t *msg);

	/**
	 * List CA sections to stroke console.
	 *
	 * @param msg		stroke message
	 */
	void (*list)(stroke_ca_t *this, stroke_msg_t *msg, FILE *out);

	/**
	 * Check if a certificate can be made available through hash and URL.
	 *
	 * @param cert		peer certificate
	 */
	void (*check_for_hash_and_url)(stroke_ca_t *this, certificate_t* cert);

	/**
	 * Destroy a stroke_ca instance.
	 */
	void (*destroy)(stroke_ca_t *this);
};

/**
 * Create a stroke_ca instance.
 */
stroke_ca_t *stroke_ca_create(stroke_cred_t *cred);

#endif /** STROKE_CA_H_ @}*/

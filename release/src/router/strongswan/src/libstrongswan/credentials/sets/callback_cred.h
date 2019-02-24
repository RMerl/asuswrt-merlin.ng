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

/**
 * @defgroup callback_cred callback_cred
 * @{ @ingroup sets
 */

#ifndef CALLBACK_CRED_H_
#define CALLBACK_CRED_H_

typedef struct callback_cred_t callback_cred_t;

#include <credentials/credential_set.h>

/**
 * Callback function to get shared keys.
 *
 * @param type			type of requested shared key
 * @param me			own identity
 * @param other			other identity
 * @param match_me		match result of own identity
 * @param match_other	match result of other identity
 */
typedef shared_key_t* (*callback_cred_shared_cb_t)(
								void *data, shared_key_type_t type,
								identification_t *me, identification_t *other,
								id_match_t *match_me, id_match_t *match_other);

/**
 * Generic callbcack using user specified callback functions.
 */
struct callback_cred_t {

	/**
	 * Implements credential_set_t.
	 */
	credential_set_t set;

	/**
	 * Destroy a callback_cred_t.
	 */
	void (*destroy)(callback_cred_t *this);
};

/**
 * Create a callback_cred instance, for a shared key.
 *
 * @param cb		callback function
 * @param data		data to pass to callback
 */
callback_cred_t *callback_cred_create_shared(callback_cred_shared_cb_t cb,
											 void *data);

#endif /** CALLBACK_CRED_H_ @}*/

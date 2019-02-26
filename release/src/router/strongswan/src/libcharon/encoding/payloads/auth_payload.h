/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup auth_payload auth_payload
 * @{ @ingroup payloads
 */

#ifndef AUTH_PAYLOAD_H_
#define AUTH_PAYLOAD_H_

typedef struct auth_payload_t auth_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <sa/authenticator.h>

/**
 * Class representing an IKEv2 AUTH payload.
 *
 * The AUTH payload format is described in RFC section 3.8.
 */
struct auth_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Set the AUTH method.
	 *
	 * @param method		auth_method_t to use
	 */
	void (*set_auth_method) (auth_payload_t *this, auth_method_t method);

	/**
	 * Get the AUTH method.
	 *
	 * @return				auth_method_t used
	 */
	auth_method_t (*get_auth_method) (auth_payload_t *this);

	/**
	 * Set the AUTH data.
	 *
	 * @param data			AUTH data as chunk_t, gets cloned
	 */
	void (*set_data) (auth_payload_t *this, chunk_t data);

	/**
	 * Get the AUTH data.
	 *
	 * @return				AUTH data as chunk_t, internal data
	 */
	chunk_t (*get_data) (auth_payload_t *this);

	/**
	 * Get the value of a reserved bit.
	 *
	 * @param nr			number of the reserved bit, 0-6
	 * @return				TRUE if bit was set, FALSE to clear
	 */
	bool (*get_reserved_bit)(auth_payload_t *this, u_int nr);

	/**
	 * Set one of the reserved bits.
	 *
	 * @param nr			number of the reserved bit, 0-6
	 */
	void (*set_reserved_bit)(auth_payload_t *this, u_int nr);

	/**
	 * Destroys an auth_payload_t object.
	 */
	void (*destroy) (auth_payload_t *this);
};

/**
 * Creates an empty auth_payload_t object.
 *
 * @return auth_payload_t object
 */
auth_payload_t *auth_payload_create(void);

#endif /** AUTH_PAYLOAD_H_ @}*/

/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup eap_payload eap_payload
 * @{ @ingroup payloads
 */

#ifndef EAP_PAYLOAD_H_
#define EAP_PAYLOAD_H_

typedef struct eap_payload_t eap_payload_t;

#include <library.h>
#include <eap/eap.h>
#include <encoding/payloads/payload.h>

/**
 * Class representing an IKEv2 EAP payload.
 *
 * The EAP payload format is described in RFC section 3.16.
 */
struct eap_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Set the contained EAP data.
	 *
	 * This contains the FULL EAP message starting with "code".
	 * Chunk gets cloned.
	 *
	 * @param message	EAP data
	 */
	void (*set_data) (eap_payload_t *this, chunk_t data);

	/**
	 * Get the contained EAP data.
	 *
	 * This contains the FULL EAP message starting with "code".
	 *
	 * @return			EAP data (pointer to internal data)
	 */
	chunk_t (*get_data) (eap_payload_t *this);

	/**
	 * Get the EAP code.
	 *
	 * @return			EAP message as chunk_t
	 */
	eap_code_t (*get_code) (eap_payload_t *this);

	/**
	 * Get the EAP identifier.
	 *
	 * @return			unique identifier
	 */
	u_int8_t (*get_identifier) (eap_payload_t *this);

	/**
	 * Get the EAP method type.
	 *
	 * @param vendor	pointer receiving vendor identifier
	 * @return			EAP method type, vendor specific if vendor != 0
	 */
	eap_type_t (*get_type) (eap_payload_t *this, u_int32_t *vendor);

	/**
	 * Enumerate the EAP method types contained in an EAP-Nak (i.e. get_type()
	 * returns EAP_NAK).
	 *
	 * @return			enumerator over (eap_type_t type, u_int32_t vendor)
	 */
	enumerator_t* (*get_types) (eap_payload_t *this);

	/**
	 * Check if the EAP method type is encoded in the Expanded Type format.
	 *
	 * @return			TRUE if in Expanded Type format
	 */
	bool (*is_expanded) (eap_payload_t *this);

	/**
	 * Destroys an eap_payload_t object.
	 */
	void (*destroy) (eap_payload_t *this);
};

/**
 * Creates an empty eap_payload_t object.
 *
 * @return 			eap_payload_t object
 */
eap_payload_t *eap_payload_create(void);

/**
 * Creates an eap_payload_t object with data.
 *
 * @param data		data, gets cloned
 * @return 			eap_payload_t object
 */
eap_payload_t *eap_payload_create_data(chunk_t data);

/**
 * Creates an eap_payload_t object with data, owning the data.
 *
 * @param data		data on heap, gets owned and freed
 * @return 			eap_payload_t object
 */
eap_payload_t *eap_payload_create_data_own(chunk_t data);

/**
 * Creates an eap_payload_t object with a code.
 *
 * Could should be either EAP_SUCCESS/EAP_FAILURE, use
 * constructor above otherwise.
 *
 * @param code			EAP status code
 * @param identifier	EAP identifier to use in payload
 * @return 				eap_payload_t object
 */
eap_payload_t *eap_payload_create_code(eap_code_t code, u_int8_t identifier);

/**
 * Creates an eap_payload_t EAP_RESPONSE containing an EAP_NAK.
 *
 * @param identifier	EAP identifier to use in payload
 * @param type			preferred auth type, 0 to send all supported types
 * @param vendor		vendor identifier for auth type, 0 for default
 * @param expanded		TRUE to send an expanded Nak
 * @return 				eap_payload_t object
 */
eap_payload_t *eap_payload_create_nak(u_int8_t identifier, eap_type_t type,
									  u_int32_t vendor, bool expanded);

#endif /** EAP_PAYLOAD_H_ @}*/

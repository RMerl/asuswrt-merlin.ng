/*
 * Copyright (C) 2007 Tobias Brunner
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
 * @defgroup id_payload id_payload
 * @{ @ingroup payloads
 */

#ifndef ID_PAYLOAD_H_
#define ID_PAYLOAD_H_

typedef struct id_payload_t id_payload_t;

#include <library.h>
#include <utils/identification.h>
#include <encoding/payloads/payload.h>
#include <selectors/traffic_selector.h>

/**
 * Object representing an IKEv1 or an IKEv2 ID payload.
 */
struct id_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Creates an identification object of this id payload.
	 *
	 * @return				identification_t object
	 */
	identification_t *(*get_identification) (id_payload_t *this);

	/**
	 * Creates a traffic selector form a ID_ADDR_SUBNET/RANGE identity.
	 *
	 * @return				traffic selector, NULL on failure
	 */
	traffic_selector_t* (*get_ts)(id_payload_t *this);

	/**
	 * Get encoded payload without fixed payload header (used for IKEv1).
	 *
	 * @return				encoded payload (gets allocated)
	 */
	chunk_t (*get_encoded)(id_payload_t *this);

	/**
	 * Destroys an id_payload_t object.
	 */
	void (*destroy) (id_payload_t *this);
};

/**
 * Creates an empty id_payload_t object.
 *
 * @param type		one of PLV2_ID_INITIATOR, PLV2_ID_RESPONDER, PLV1_ID and PLV1_NAT_OA
 * @return			id_payload_t object
 */
id_payload_t *id_payload_create(payload_type_t type);

/**
 * Creates an id_payload_t from an existing identification_t object.
 *
 * @param type		one of PLV2_ID_INITIATOR, PLV2_ID_RESPONDER, PLV1_ID and PLV1_NAT_OA
 * @param id		identification_t object
 * @return			id_payload_t object
 */
id_payload_t *id_payload_create_from_identification(payload_type_t type,
													identification_t *id);

/**
 * Create an IKEv1 ID_ADDR_SUBNET/RANGE identity from a traffic selector.
 *
 * @param ts		traffic selector
 * @return			PLV1_ID id_paylad_t object.
 */
id_payload_t *id_payload_create_from_ts(traffic_selector_t *ts);

#endif /** ID_PAYLOAD_H_ @}*/

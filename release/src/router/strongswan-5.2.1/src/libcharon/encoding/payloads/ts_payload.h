/*
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
 * @defgroup ts_payload ts_payload
 * @{ @ingroup payloads
 */

#ifndef TS_PAYLOAD_H_
#define TS_PAYLOAD_H_

typedef struct ts_payload_t ts_payload_t;

#include <library.h>
#include <collections/linked_list.h>
#include <selectors/traffic_selector.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/traffic_selector_substructure.h>

/**
 * Class representing an IKEv2 TS payload.
 *
 * The TS payload format is described in RFC section 3.13.
 */
struct ts_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the type of TSpayload (TSi or TSr).
	 *
	 * @return
	 * 						- TRUE if this payload is of type TSi
	 * 						- FALSE if this payload is of type TSr
	 */
	bool (*get_initiator) (ts_payload_t *this);

	/**
	 * Set the type of TS payload (TSi or TSr).
	 *
	 * @param is_initiator
	 * 						- TRUE if this payload is of type TSi
	 * 						- FALSE if this payload is of type TSr
	 */
	void (*set_initiator) (ts_payload_t *this,bool is_initiator);

	/**
	 * Get a list of nested traffic selectors as traffic_selector_t.
	 *
	 * Resulting list and its traffic selectors must be destroyed after usage
	 *
	 * @return				list of traffic selectors
	 */
	linked_list_t *(*get_traffic_selectors) (ts_payload_t *this);

	/**
	 * Destroys an ts_payload_t object.
	 */
	void (*destroy) (ts_payload_t *this);
};

/**
 * Creates an empty ts_payload_t object.
 *
 * @param is_initiator		TRUE for TSi, FALSE for TSr payload type
 * @return					ts_payload_t object
 */
ts_payload_t *ts_payload_create(bool is_initiator);

/**
 * Creates ts_payload with a list of traffic_selector_t
 *
 * @param is_initiator		TRUE for TSi, FALSE for TSr payload type
 * @param traffic_selectors	list of traffic selectors to include
 * @return					ts_payload_t object
 */
ts_payload_t *ts_payload_create_from_traffic_selectors(bool is_initiator,
											linked_list_t *traffic_selectors);

#endif /** TS_PAYLOAD_H_ @}*/

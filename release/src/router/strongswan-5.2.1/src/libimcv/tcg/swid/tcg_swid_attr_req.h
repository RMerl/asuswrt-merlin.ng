/*
 * Copyright (C) 2013-2014 Andreas Steffen
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
 * @defgroup tcg_swid_attr_req tcg_swid_attr_req
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_SWID_ATTR_REQ_H_
#define TCG_SWID_ATTR_REQ_H_

#define TCG_SWID_REQ_MIN_SIZE	12

typedef struct tcg_swid_attr_req_t tcg_swid_attr_req_t;
typedef enum tcg_swid_attr_req_flag_t tcg_swid_attr_req_flag_t;

enum tcg_swid_attr_req_flag_t {
	TCG_SWID_ATTR_REQ_FLAG_NONE = 0,
	TCG_SWID_ATTR_REQ_FLAG_R =   (1 << 7),
	TCG_SWID_ATTR_REQ_FLAG_S =   (1 << 6),
	TCG_SWID_ATTR_REQ_FLAG_C =   (1 << 5)
};

#include "tcg/tcg_attr.h"
#include "swid/swid_tag_id.h"
#include "swid/swid_inventory.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the TCG SWID Request attribute
 */
struct tcg_swid_attr_req_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get SWID request flags
	 *
	 * @return				Flags
	 */
	u_int8_t (*get_flags)(tcg_swid_attr_req_t *this);

	/**
	 * Get Request ID
	 *
	 * @return				Request ID
	 */
	u_int32_t (*get_request_id)(tcg_swid_attr_req_t *this);

	/**
	 * Get Earliest EID
	 *
	 * @return				Event ID
	 */
	u_int32_t (*get_earliest_eid)(tcg_swid_attr_req_t *this);

	/**
	 * Add Tag ID
	 *
	 * @param tag_id			SWID Tag ID (is not cloned by constructor!)
	 */
	void (*add_target)(tcg_swid_attr_req_t *this, swid_tag_id_t *tag_id);

	/**
	 * Create Tag ID enumerator
	 *
	 * @return					Get a list of target tag IDs
	 */
	swid_inventory_t* (*get_targets)(tcg_swid_attr_req_t *this);

};

/**
 * Creates an tcg_swid_attr_req_t object
 *
 * @param flags				Sets the C|S|R flags
 * @param request_id		Request ID
 * @param eid				Earliest Event ID
 */
pa_tnc_attr_t* tcg_swid_attr_req_create(u_int8_t flags, u_int32_t request_id,
										u_int32_t eid);

/**
 * Creates an tcg_swid_attr_req_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_swid_attr_req_create_from_data(size_t length, chunk_t value);

#endif /** TCG_SWID_ATTR_REQ_H_ @}*/

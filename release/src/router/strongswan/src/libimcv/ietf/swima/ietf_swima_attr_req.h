/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup ietf_swima_attr_req ietf_swima_attr_req
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_SWIMA_ATTR_REQ_H_
#define IETF_SWIMA_ATTR_REQ_H_

#define IETF_SWIMA_REQ_MIN_SIZE	12

typedef struct ietf_swima_attr_req_t ietf_swima_attr_req_t;
typedef enum ietf_swima_attr_req_flag_t ietf_swima_attr_req_flag_t;

enum ietf_swima_attr_req_flag_t {
	IETF_SWIMA_ATTR_REQ_FLAG_NONE = 0,
	IETF_SWIMA_ATTR_REQ_FLAG_C =   (1 << 7),
	IETF_SWIMA_ATTR_REQ_FLAG_S =   (1 << 6),
	IETF_SWIMA_ATTR_REQ_FLAG_R =   (1 << 5)
};

#include "swima/swima_inventory.h"
#include "ietf/ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the IETF SW Request attribute
 */
struct ietf_swima_attr_req_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get SW request flags
	 *
	 * @return				Flags
	 */
	uint8_t (*get_flags)(ietf_swima_attr_req_t *this);

	/**
	 * Get Request ID
	 *
	 * @return				Request ID
	 */
	uint32_t (*get_request_id)(ietf_swima_attr_req_t *this);

	/**
	 * Set Software Identity targets
	 *
	 * @param targets		SW ID inventory containing targets (not cloned)
	 */
	void (*set_targets)(ietf_swima_attr_req_t *this, swima_inventory_t *targets);

	/**
	 * Get Software Identity targets
	 *
	 * @return				SW ID inventory containing targets
	 */
	swima_inventory_t* (*get_targets)(ietf_swima_attr_req_t *this);

};

/**
 * Creates an ietf_swima_attr_req_t object
 *
 * @param flags				Sets the C|S|R flags
 * @param request_id		Request ID
 */
pa_tnc_attr_t* ietf_swima_attr_req_create(uint8_t flags, uint32_t request_id);

/**
 * Creates an ietf_swima_attr_req_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_swima_attr_req_create_from_data(size_t length, chunk_t value);

#endif /** IETF_SWIMA_ATTR_REQ_H_ @}*/

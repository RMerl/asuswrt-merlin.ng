/*
 * Copyright (C) 2017 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup ietf_swima_attr_sw_ev ietf_swima_attr_sw_ev
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_SWIMA_ATTR_SW_EV_H_
#define IETF_SWIMA_ATTR_SW_EV_H_

#define IETF_SWIMA_SW_EV_MIN_SIZE	20

typedef struct ietf_swima_attr_sw_ev_t ietf_swima_attr_sw_ev_t;
typedef enum ietf_swima_attr_sw_ev_flag_t ietf_swima_attr_sw_ev_flag_t;

enum ietf_swima_attr_sw_ev_flag_t {
	IETF_SWIMA_ATTR_SW_EV_FLAG_NONE =   0,
	IETF_SWIMA_ATTR_SW_EV_FLAG_S_F  =  (1 << 7)
};

#include "ietf/ietf_attr.h"
#include "swima/swima_events.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the IETF SW Identifier Inventory attribute
 *
 */
struct ietf_swima_attr_sw_ev_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get Software Inventory flags
	 *
	 * @return				Flags
	 */
	uint8_t (*get_flags)(ietf_swima_attr_sw_ev_t *this);

	/**
	 * Get Request ID
	 *
	 * @return				Request ID
	 */
	uint32_t (*get_request_id)(ietf_swima_attr_sw_ev_t *this);

	/**
	 * Get number of Software [Identifier] Events
	 *
	 * @return				Software [Identifier] event count
	 */
	uint32_t (*get_event_count)(ietf_swima_attr_sw_ev_t *this);

	/**
	 * Add Software [Identifier] Events
	 *
	 * @param sw_events		List of Software [Identifier] events to be added
	 */
	void (*set_events)(ietf_swima_attr_sw_ev_t *this,
					   swima_events_t *sw_events);
	/**
	 * Get Software [Identifier] Events
	 *
	 * @result				Software [Identifier] events
	 */
	swima_events_t* (*get_events)(ietf_swima_attr_sw_ev_t *this);

	/**
	 * Remove all Software [Identifier] events
	 */
	void (*clear_events)(ietf_swima_attr_sw_ev_t *this);

};

/**
 * Creates an ietf_swima_attr_sw_ev_t object
 *
 * @param flags				Sets the flags
 * @param request_id		Copy of the Request ID
 * @param sw_id_only		TRUE if the Software ID, only is transmitted
 */
pa_tnc_attr_t* ietf_swima_attr_sw_ev_create(uint8_t flags, uint32_t request_id,
											bool sw_id_only);

/**
 * Creates an ietf_swima_attr_sw_ev_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 * @param sw_id_only		TRUE if the Software ID, only is transmitted
 */
pa_tnc_attr_t* ietf_swima_attr_sw_ev_create_from_data(size_t length,
										chunk_t value, bool sw_id_only);

#endif /** IETF_SWIMA_ATTR_SW_EV_H_ @}*/

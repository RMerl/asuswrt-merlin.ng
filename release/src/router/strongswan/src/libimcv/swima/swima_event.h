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
 * @defgroup swima_event swima_event
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_EVENT_H_
#define SWIMA_EVENT_H_

#include "swima_record.h"

#include <library.h>

#define SWIMA_EVENT_ACTION_NONE			0
#define SWIMA_EVENT_ACTION_CREATION		1
#define SWIMA_EVENT_ACTION_DELETION		2
#define SWIMA_EVENT_ACTION_ALTERATION	3
#define SWIMA_EVENT_ACTION_LAST			3

typedef struct swima_event_t swima_event_t;

/**
 * Class storing a Software [Identifier] event
 */
struct swima_event_t {

	/**
	 * Get Event ID and optionally the associated timestamp
	 *
	 * @param timestamp		Timestamp associated with Event
	 * @return				Event ID
	 */
	uint32_t (*get_eid)(swima_event_t *this, chunk_t *timestamp);

	/**
	 * Get Action associated with Event
	 *
	 * @return				Action associated with event
	 */
	uint8_t (*get_action)(swima_event_t *this);

	/**
	 * Get Software [Identifier] record
	 *
	 * @return				Software [Identifier] record
	 */
	swima_record_t* (*get_sw_record)(swima_event_t *this);

	/**
	 * Get a new reference to a swima_event object
	 *
	 * @return			this, with an increased refcount
	 */
	swima_event_t* (*get_ref)(swima_event_t *this);

	/**
	 * Destroys a swima_event_t object.
	 */
	void (*destroy)(swima_event_t *this);

};

/**
 * Creates a swima_event_t object
 *
 * @param eid				Event ID
 * @param timestamp			Time of Event
 * @param action			Action (CREATION, DELETION, ALTERATION)
 * @param sw_record			Software [Identifier] record
 */
swima_event_t* swima_event_create(uint32_t eid, chunk_t timestamp,
								  uint8_t action, swima_record_t *sw_record);

#endif /** SWIMA_EVENT_H_ @}*/

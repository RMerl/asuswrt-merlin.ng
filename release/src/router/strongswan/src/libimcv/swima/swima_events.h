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
 * @defgroup swima_events swima_events
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_EVENTS_H_
#define SWIMA_EVENTS_H_

#define SWIMA_MAX_ATTR_SIZE	10000000

#include "swima_event.h"

#include <library.h>

typedef struct swima_events_t swima_events_t;

/**
 * Class managing list of Software [Identifier] Events
 */
struct swima_events_t {

	/**
	 * Add event to list
	 *
	 * @param event		Event to be added
	 */
	void (*add)(swima_events_t *this, swima_event_t *event);

	/**
	 * Get the number of events in the event list
	 *
	 * @return				Number of events
	 */
	int (*get_count)(swima_events_t *this);

	/**
	 * Set both the Last and Last Consulted Event ID
	 *
	 * @param				Last [Consulted] Event ID
	 * @param				Epoch of event IDs
	 */
	void (*set_eid)(swima_events_t *this, uint32_t eid, uint32_t epoch);

	/**
	 * Set Last Event ID if different from Last Consulted Event ID
	 *
	 * @param last_eid		Last Event ID
	 */
	void (*set_last_eid)(swima_events_t *this, uint32_t last_eid);

	/**
	 * Get both the Last and Last Consulted Event ID
	 *
	 * @param eid_epoch		Event ID Epoch
	 * @param last_eid		Last Event ID
	 * @return				Last Consulted Event ID
	 */
	uint32_t (*get_eid)(swima_events_t *this, uint32_t *epoch, uint32_t *last_eid);

	/**
	  * Create an event enumerator
	  *
	  * @return				Enumerator returning events
	  */
	enumerator_t* (*create_enumerator)(swima_events_t *this);

	/**
	 * Get a new reference to a swima_events object
	 *
	 * @return			this, with an increased refcount
	 */
	swima_events_t* (*get_ref)(swima_events_t *this);

	/**
	 * Clears the events, keeping the eid and epoch values.
	 */
	void (*clear)(swima_events_t *this);

	/**
	 * Destroys a swima_events_t object.
	 */
	void (*destroy)(swima_events_t *this);

};

/**
 * Creates a swima_events_t object
 */
swima_events_t* swima_events_create(void);

#endif /** SWIMA_EVENTS_H_ @}*/

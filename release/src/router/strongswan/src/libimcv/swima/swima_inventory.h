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
 * @defgroup swima_inventory swima_inventory
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_INVENTORY_H_
#define SWIMA_INVENTORY_H_

#define SWIMA_MAX_ATTR_SIZE	10000000

#include "swima_record.h"

#include <library.h>

typedef struct swima_inventory_t swima_inventory_t;

/**
 * Class managing software inventory
 */
struct swima_inventory_t {

	/**
	 * Add evidence record to software inventory
	 *
	 * @param record		Software evidence record to be added
	 */
	void (*add)(swima_inventory_t *this, swima_record_t *record);

	/**
	 * Get the number of evidence records in the software inventory
	 *
	 * @return				Number evidence records
	 */
	int (*get_count)(swima_inventory_t *this);

	/**
	 * Set the earliest or last event ID of the inventory
	 *
	 * @param				Event ID
	 * @param				Epoch of event IDs
	 */
	void (*set_eid)(swima_inventory_t *this, uint32_t eid, uint32_t epoch);

	/**
	 * Get the earliest or last event ID of the inventory
	 *
	 * @param				Epoch of event IDs
	 * @return				Event ID
	 */
	uint32_t (*get_eid)(swima_inventory_t *this, uint32_t *epoch);

	/**
	  * Create a software inventory evidence record enumerator
	  *
	  * @return				Enumerator returning evidence records
	  */
	enumerator_t* (*create_enumerator)(swima_inventory_t *this);

	/**
	 * Get a new reference to a swima_inventory object
	 *
	 * @return				This, with an increased refcount
	 */
	swima_inventory_t* (*get_ref)(swima_inventory_t *this);

	/**
	 * Clears the inventory, keeping the eid and epoch values
	 */
	void (*clear)(swima_inventory_t *this);

	/**
	 * Destroys a swima_inventory_t object
	 */
	void (*destroy)(swima_inventory_t *this);

};

/**
 * Creates a swima_inventory_t object
 *
 */
swima_inventory_t* swima_inventory_create(void);

#endif /** SWIMA_INVENTORY_H_ @}*/

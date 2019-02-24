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
 * @defgroup swima_collector swima_collector
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_COLLECTOR_H_
#define SWIMA_COLLECTOR_H_

#include "swima/swima_inventory.h"
#include "swima/swima_events.h"

typedef struct swima_collector_t swima_collector_t;

/**
 * Class collecting Software [Identity] Inventory
 */
struct swima_collector_t {

	/**
	 * Collect the Software [Identity] Inventory
	 *
	 * @param sw_id_only	TRUE to request Software Identity Inventory only
	 * @param targets		Software Identity targets
	 * @return				Software [Identity] Inventory
	 */
	swima_inventory_t* (*collect_inventory)(swima_collector_t *this,
											bool sw_id_only, 
											swima_inventory_t *targets);

	/**
	 * Collect Software [Identity] Events
	 *
	 * @param sw_id_only	TRUE to request Software Identity Inventory only
	 * @param targets		Software Identity targets
	 * @return				Software [Identity] Events
	 */
	swima_events_t* (*collect_events)(swima_collector_t *this,
									  bool sw_id_only,
									  swima_inventory_t *targets);

	/**
	 * Destroys a swima_collector_t object.
	 */
	void (*destroy)(swima_collector_t *this);

};

/**
 * Creates a swima_collector_t object
 */
swima_collector_t* swima_collector_create(void);

#endif /** SWIMA_COLLECTOR_H_ @}*/

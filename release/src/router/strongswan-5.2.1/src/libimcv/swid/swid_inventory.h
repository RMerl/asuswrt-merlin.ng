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
 * @defgroup swid_inventory swid_inventory
 * @{ @ingroup libimcv_swid
 */

#ifndef SWID_INVENTORY_H_
#define SWID_INVENTORY_H_

#include <library.h>

/* Maximum size of a SWID Tag Inventory: 100 MB */
#define SWID_MAX_ATTR_SIZE	100000000

typedef struct swid_inventory_t swid_inventory_t;

/**
 * Class managing SWID tag inventory
 */
struct swid_inventory_t {

	/**
	 * Collect the SWID tags stored on the endpoint
	 *
	 * @param directory		SWID directory path
	 * @param generator		Path to SWID generator
	 * @param targets		List of target tag IDs
	 * @param pretty		Generate indented XML SWID tags
	 * @param full			Include file information in SWID tags
	 * @return				TRUE if successful
	 */
	bool (*collect)(swid_inventory_t *this, char *directory, char *generator,
					swid_inventory_t *targets, bool pretty, bool full);

	/**
	 * Collect the SWID tags stored on the endpoint
	 *
	 * @param item			SWID tag or tag ID to be added
	 */
	void (*add)(swid_inventory_t *this, void *item);

	/**
	 * Get the number of collected SWID tags
	 *
	 * @return				Number of collected SWID tags
	 */
	int (*get_count)(swid_inventory_t *this);

	/**
	  * Create a SWID tag inventory enumerator
	  *
	  * @return				Enumerator returning either tag ID or full tag
	  */
	enumerator_t* (*create_enumerator)(swid_inventory_t *this);

	/**
	 * Destroys a swid_inventory_t object.
	 */
	void (*destroy)(swid_inventory_t *this);

};

/**
 * Creates a swid_inventory_t object
 *
 * @param full_tags			TRUE if full tags, FALSE if tag IDs only
 */
swid_inventory_t* swid_inventory_create(bool full_tags);

#endif /** SWID_INVENTORY_H_ @}*/

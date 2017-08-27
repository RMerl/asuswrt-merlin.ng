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
 * @defgroup tcg_swid_attr_tag_inv tcg_swid_attr_tag_inv
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_SWID_ATTR_TAG_INV_H_
#define TCG_SWID_ATTR_TAG_INV_H_

typedef struct tcg_swid_attr_tag_inv_t tcg_swid_attr_tag_inv_t;

#include "tcg/tcg_attr.h"
#include "swid/swid_tag.h"
#include "swid/swid_inventory.h"

#include <pa_tnc/pa_tnc_attr.h>

#define TCG_SWID_TAG_INV_MIN_SIZE	16

/**
 * Class implementing the TCG SWID Tag Inventory attribute
 *
 */
struct tcg_swid_attr_tag_inv_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Add a Tag ID to the attribute
	 *
	 * @param tag				SWID Tag to be added
	 */
	void (*add)(tcg_swid_attr_tag_inv_t *this, swid_tag_t *tag);
	/**
	 * Get Request ID
	 *
	 * @return					Request ID
	 */
	uint32_t (*get_request_id)(tcg_swid_attr_tag_inv_t *this);

	/**
	 * Get Last Event ID
	 *
	 * @param eid_epoch			Event ID Epoch
	 * @return					Last Event ID
	 */
	uint32_t (*get_last_eid)(tcg_swid_attr_tag_inv_t *this,
							  uint32_t *eid_epoch);

	/**
	 * Get count of remaining SWID tags
	 *
	 * @return					SWID Tag count
	 */
	uint32_t (*get_tag_count)(tcg_swid_attr_tag_inv_t *this);

	/**
	 * Get Inventory of SWID tags
	 *
	 * @result					SWID Tag Inventory
	 */
	swid_inventory_t* (*get_inventory)(tcg_swid_attr_tag_inv_t *this);

	/**
	 * Remove all SWID Tags from the Inventory
	 */
	void (*clear_inventory)(tcg_swid_attr_tag_inv_t *this);

};

/**
 * Creates an tcg_swid_attr_tag_inv_t object
 *
 * @param request_id			Copy of the Request ID
 * @param eid_epoch				Event ID Epoch
 * @param eid					Last Event ID
 */
pa_tnc_attr_t* tcg_swid_attr_tag_inv_create(uint32_t request_id,
											uint32_t eid_epoch,
											uint32_t eid);

/**
 * Creates an tcg_swid_attr_tag_inv_t object from received data
 *
 * @param length				Total length of attribute value
 * @param value					Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_swid_attr_tag_inv_create_from_data(size_t length,
													  chunk_t value);

#endif /** TCG_SWID_ATTR_TAG_INV_H_ @}*/

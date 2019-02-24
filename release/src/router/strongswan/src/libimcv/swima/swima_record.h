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
 * @defgroup swima_record swima_record
 * @{ @ingroup libimcv_swima
 */

#ifndef SWIMA_RECORD_H_
#define SWIMA_RECORD_H_

#include <library.h>
#include <pen/pen.h>

typedef struct swima_record_t swima_record_t;

/**
 * Class storing a Software Inventory Evidence Collection record
 */
struct swima_record_t {

	/**
	 * Get Software Identifier and optional Software Location
	 *
	 * @return				Record ID
	 */
	uint32_t (*get_record_id)(swima_record_t *this);

	/**
	 * Get Software Identifier and optional Software Location
	 *
	 * @param sw_locator	Optional Software Locator
	 * @return				Software Identifier
	 */
	chunk_t (*get_sw_id)(swima_record_t *this, chunk_t *sw_locator);

	/**
	 * Set Data Model
	 *
	 * @param				Data model type in PEN namespace
	 */
	void (*set_data_model)(swima_record_t *this, pen_type_t data_model);

	/**
	 * Get Data Model
	 *
	 * @return				Data model type in PEN namespace
	 */
	pen_type_t (*get_data_model)(swima_record_t *this);

	/**
	 * Set Source ID
	 *
	 * @param				Source ID
	 */
	void (*set_source_id)(swima_record_t *this, uint8_t source_id);

	/**
	 * Get Source ID
	 *
	 * @return				Source ID
	 */
	uint8_t (*get_source_id)(swima_record_t *this);

	/**
	 * Set Software Inventory Evidence Record
	 *
	 * @param				Software Inventory Evidence Record
	 */
	void (*set_record)(swima_record_t *this, chunk_t record);

	/**
	 * Get Software Inventory Evidence Record
	 *
	 * @return				Software Inventory Evidence Record
	 */
	chunk_t (*get_record)(swima_record_t *this);

	/**
	 * Get a new reference to a swima_record object
	 *
	 * @return			this, with an increased refcount
	 */
	swima_record_t* (*get_ref)(swima_record_t *this);

	/**
	 * Destroys a swima_record_t object.
	 */
	void (*destroy)(swima_record_t *this);

};

/**
 * Creates a swima_record_t object
 *
 * @param record_id			Record ID
 * @param sw_id				Software Identifierl
 * @param sw_locator		Software Locator or empty chunk
 */
swima_record_t* swima_record_create(uint32_t record_id, chunk_t sw_id,
									chunk_t sw_locator);

#endif /** SWIMA_RECORD_H_ @}*/

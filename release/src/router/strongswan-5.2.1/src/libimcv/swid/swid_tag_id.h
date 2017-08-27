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
 * @defgroup swid_tag_id swid_tag_id
 * @{ @ingroup libimcv_swid
 */

#ifndef SWID_TAG_ID_H_
#define SWID_TAG_ID_H_

#include <library.h>

typedef struct swid_tag_id_t swid_tag_id_t;


/**
 * Class storing a SWID Tag ID
 */
struct swid_tag_id_t {

	/**
	 * Get the Tag Creator
	 *
	 * @return				Tag Creator
	 */
	chunk_t (*get_tag_creator)(swid_tag_id_t *this);

	/**
	 * Get the Unique Software ID and optional Tag File Path
	 *
	 * @param instance_id	Optional Tag Identifier Instance ID
	 * @return				Unique Software ID
	 */
	chunk_t (*get_unique_sw_id)(swid_tag_id_t *this, chunk_t *instance_id);

	/**
	 * Get a new reference to the swid_tag_id object
	 *
	 * @return			this, with an increased refcount
	 */
	swid_tag_id_t* (*get_ref)(swid_tag_id_t *this);

	/**
	 * Destroys a swid_tag_id_t object.
	 */
	void (*destroy)(swid_tag_id_t *this);

};

/**
 * Creates a swid_tag_id_t object
 *
 * @param tag_creator		Tag Creator
 * @param unique_sw_id		Unique Software ID
 * @param instance_id		Tag Identifier Instance ID or empty chunk
 */
swid_tag_id_t* swid_tag_id_create(chunk_t tag_creator, chunk_t unique_sw_id,
								  chunk_t instance_id);

#endif /** SWID_TAG_ID_H_ @}*/

/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup transform_substructure transform_substructure
 * @{ @ingroup payloads
 */

#ifndef TRANSFORM_SUBSTRUCTURE_H_
#define TRANSFORM_SUBSTRUCTURE_H_

typedef struct transform_substructure_t transform_substructure_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_attribute.h>
#include <collections/linked_list.h>
#include <crypto/diffie_hellman.h>
#include <crypto/signers/signer.h>
#include <crypto/prfs/prf.h>
#include <crypto/crypters/crypter.h>
#include <crypto/proposal/proposal.h>

/**
 * IKEv1 Value for a transform payload.
 */
#define TRANSFORM_TYPE_VALUE 3

/**
 * Class representing an IKEv1/IKEv2 transform substructure.
 */
struct transform_substructure_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Adds a transform_attribute_t object to this object.
	 *
	 * @param proposal  transform_attribute_t object to add
	 */
	void (*add_transform_attribute) (transform_substructure_t *this,
									 transform_attribute_t *attribute);

	/**
	 * Sets the next_payload field of this substructure
	 *
	 * If this is the last transform, next payload field is set to 0,
	 * otherwise to 3
	 *
	 * @param is_last	When TRUE, next payload field is set to 0, otherwise to 3
	 */
	void (*set_is_last_transform) (transform_substructure_t *this, bool is_last);

	/**
	 * Get transform type (IKEv2) or the transform number (IKEv1).
	 *
	 * @return 			Transform type of current transform substructure.
	 */
	uint8_t (*get_transform_type_or_number) (transform_substructure_t *this);

	/**
	 * Get transform id of the current transform.
	 *
	 * @return 			Transform id of current transform substructure.
	 */
	uint16_t (*get_transform_id) (transform_substructure_t *this);

	/**
	 * Create an enumerator over transform attributes.
	 *
	 * @return			enumerator over transform_attribute_t*
	 */
	enumerator_t* (*create_attribute_enumerator)(transform_substructure_t *this);

	/**
	 * Destroys an transform_substructure_t object.
	 */
	void (*destroy) (transform_substructure_t *this);
};

/**
 * Creates an empty transform_substructure_t object.
 *
 * @param type			PLV2_TRANSFORM_SUBSTRUCTURE or PLV1_TRANSFORM_SUBSTRUCTURE
 * @return				created transform_substructure_t object
 */
transform_substructure_t *transform_substructure_create(payload_type_t type);

/**
 * Creates an empty transform_substructure_t object.
 *
 * @param type				PLV2_TRANSFORM_SUBSTRUCTURE or PLV1_TRANSFORM_SUBSTRUCTURE
 * @param type_or_number	Type (IKEv2) or number (IKEv1) of transform
 * @param id				transform id specific for the transform type
 * @return					transform_substructure_t object
 */
transform_substructure_t *transform_substructure_create_type(payload_type_t type,
										uint8_t type_or_number, uint16_t id);

#endif /** TRANSFORM_SUBSTRUCTURE_H_ @}*/

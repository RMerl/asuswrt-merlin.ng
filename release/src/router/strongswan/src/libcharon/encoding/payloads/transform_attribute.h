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
 * @defgroup transform_attribute transform_attribute
 * @{ @ingroup payloads
 */

#ifndef TRANSFORM_ATTRIBUTE_H_
#define TRANSFORM_ATTRIBUTE_H_

typedef enum transform_attribute_type_t transform_attribute_type_t;
typedef struct transform_attribute_t transform_attribute_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Type of the attribute.
 */
enum transform_attribute_type_t {
	/** IKEv1 Phase 1 attributes */
	TATTR_PH1_ENCRYPTION_ALGORITHM = 1,
	TATTR_PH1_HASH_ALGORITHM = 2,
	TATTR_PH1_AUTH_METHOD = 3,
	TATTR_PH1_GROUP = 4,
	TATTR_PH1_GROUP_TYPE = 5,
	TATTR_PH1_GROUP_PRIME = 6,
	TATTR_PH1_GROUP_GENONE = 7,
	TATTR_PH1_GROUP_GENTWO = 8,
	TATTR_PH1_GROUP_CURVE_A = 9,
	TATTR_PH1_GROUP_CURVE_B = 10,
	TATTR_PH1_LIFE_TYPE = 11,
	TATTR_PH1_LIFE_DURATION = 12,
	TATTR_PH1_PRF = 13,
	TATTR_PH1_KEY_LENGTH = 14,
	TATTR_PH1_FIELD_SIZE = 15,
	TATTR_PH1_GROUP_ORDER = 16,
	/** IKEv1 Phase 2 attributes */
	TATTR_PH2_SA_LIFE_TYPE = 1,
	TATTR_PH2_SA_LIFE_DURATION = 2,
	TATTR_PH2_GROUP = 3,
	TATTR_PH2_ENCAP_MODE = 4,
	TATTR_PH2_AUTH_ALGORITHM = 5,
	TATTR_PH2_KEY_LENGTH = 6,
	TATTR_PH2_KEY_ROUNDS = 7,
	TATTR_PH2_COMP_DICT_SIZE = 8,
	TATTR_PH2_COMP_PRIV_ALGORITHM = 9,
	TATTR_PH2_ECN_TUNNEL = 10,
	TATTR_PH2_EXT_SEQ_NUMBER = 11,
	/* IKEv2 key length attribute */
	TATTR_IKEV2_KEY_LENGTH = 14,
	/* undefined, private use attribute */
	TATTR_UNDEFINED = 16384,
};

/**
 * Enum names for IKEv1 Phase 1 transform_attribute_type_t.
 */
extern enum_name_t *tattr_ph1_names;

/**
 * Enum names for IKEv1 Phase 2 transform_attribute_type_t.
 */
extern enum_name_t *tattr_ph2_names;

/**
 * Enum names for IKEv2 transform_attribute_type_t.
 */
extern enum_name_t *tattr_ikev2_names;


/**
 * Class representing an IKEv1/IKEv2 TRANSFORM Attribute.
 */
struct transform_attribute_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Returns the currently set value of the attribute.
	 *
	 * Returned data are not copied.
	 *
	 * @return 		chunk_t pointing to internal value
	 */
	chunk_t (*get_value_chunk) (transform_attribute_t *this);

	/**
	 * Returns the currently set value of the attribute.
	 *
	 * Returned data are not copied.
	 *
	 * @return 		value
	 */
	uint64_t (*get_value) (transform_attribute_t *this);

	/**
	 * get the type of the attribute.
	 *
	 * @return 		type of the value
	 */
	uint16_t (*get_attribute_type) (transform_attribute_t *this);

	/**
	 * Destroys an transform_attribute_t object.
	 */
	void (*destroy) (transform_attribute_t *this);
};

/**
 * Creates an empty transform_attribute_t object.
 *
 * @param type			PLV2_TRANSFORM_ATTRIBUTE or PLV1_TRANSFORM_ATTRIBUTE
 * @return				transform_attribute_t object
 */
transform_attribute_t *transform_attribute_create(payload_type_t type);

/**
 * Creates a two byte value or a larger attribute for a given attribute kind.
 *
 * @param type			PLV2_TRANSFORM_ATTRIBUTE or PLV1_TRANSFORM_ATTRIBUTE
 * @param kind			attribute kind
 * @param value			fixed two byte value
 * @return				transform_attribute_t object
 */
transform_attribute_t *transform_attribute_create_value(payload_type_t type,
							transform_attribute_type_t kind, uint64_t value);

#endif /** TRANSFORM_ATTRIBUTE_H_ @}*/

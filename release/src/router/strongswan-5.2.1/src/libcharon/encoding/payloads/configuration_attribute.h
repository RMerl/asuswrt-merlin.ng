/*
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup configuration_attribute configuration_attribute
 * @{ @ingroup payloads
 */

#ifndef CONFIGURATION_ATTRIBUTE_H_
#define CONFIGURATION_ATTRIBUTE_H_

typedef struct configuration_attribute_t configuration_attribute_t;

#include <library.h>
#include <attributes/attributes.h>
#include <encoding/payloads/payload.h>

/**
 * Class representing an IKEv2 configuration attribute / IKEv1 data attribute.
 */
struct configuration_attribute_t {

	/**
	 * Implements payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the type of the attribute.
	 *
	 * @return 		type of the configuration attribute
	 */
	configuration_attribute_type_t (*get_type)(configuration_attribute_t *this);

	/**
	 * Returns the value of the attribute as chunk.
	 *
	 * @return 		chunk_t pointing to the internal value
	 */
	chunk_t (*get_chunk) (configuration_attribute_t *this);

	/**
	 * Returns the 2 byte value of the attribute as u_int16.
	 *
	 * @return 		attribute value
	 */
	u_int16_t (*get_value) (configuration_attribute_t *this);

	/**
	 * Destroys an configuration_attribute_t object.
	 */
	void (*destroy) (configuration_attribute_t *this);
};

/**
 * Creates an empty configuration attribute.
 *
 * @param type		PLV2_CONFIGURATION_ATTRIBUTE or PLV1_CONFIGURATION_ATTRIBUTE
 * @return			created configuration attribute
 */
configuration_attribute_t *configuration_attribute_create(payload_type_t type);

/**
 * Creates a configuration attribute with type and value.
 *
 * @param type		PLV2_CONFIGURATION_ATTRIBUTE or PLV1_CONFIGURATION_ATTRIBUTE
 * @param attr_type	type of configuration attribute
 * @param chunk		attribute value, gets cloned
 * @return			created configuration attribute
 */
configuration_attribute_t *configuration_attribute_create_chunk(
	payload_type_t type, configuration_attribute_type_t attr_type, chunk_t chunk);

/**
 * Creates a IKEv1 configuration attribute with 2 bytes value (IKEv1 only).
 *
 * @param attr_type	type of configuration attribute
 * @param value		attribute value, gets cloned
 * @return			created PLV1_CONFIGURATION_ATTRIBUTE configuration attribute
 */
configuration_attribute_t *configuration_attribute_create_value(
					configuration_attribute_type_t attr_type, u_int16_t value);

#endif /** CONFIGURATION_ATTRIBUTE_H_ @}*/

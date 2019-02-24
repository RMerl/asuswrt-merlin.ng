/*
 * Copyright (C) 2005-2009 Martin Willi
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
 * @defgroup cp_payload cp_payload
 * @{ @ingroup payloads
 */

#ifndef CP_PAYLOAD_H_
#define CP_PAYLOAD_H_

typedef enum config_type_t config_type_t;
typedef struct cp_payload_t cp_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/configuration_attribute.h>
#include <collections/enumerator.h>

/**
 * Config Type of an Configuration Payload.
 */
enum config_type_t {
	CFG_REQUEST = 1,
	CFG_REPLY = 2,
	CFG_SET = 3,
	CFG_ACK = 4,
};

/**
 * enum name for config_type_t.
 */
extern enum_name_t *config_type_names;

/**
 * Class representing an IKEv2 configuration / IKEv1 attribute payload.
 */
struct cp_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Creates an enumerator of stored configuration_attribute_t objects.
	 *
	 * @return			enumerator over configration_attribute_T
	 */
	enumerator_t *(*create_attribute_enumerator) (cp_payload_t *this);

	/**
	 * Adds a configuration attribute to the configuration payload.
	 *
	 * @param attribute	attribute to add
	 */
	void (*add_attribute)(cp_payload_t *this,
						  configuration_attribute_t *attribute);

	/**
	 * Get the configuration payload type.
	 *
	 * @return			type of configuration payload
	 */
	config_type_t (*get_type) (cp_payload_t *this);

	/**
	 * Set the configuration payload identifier (IKEv1 only).
	 *
	 @param identifier	identifier to set
	 */
	void (*set_identifier) (cp_payload_t *this, uint16_t identifier);

	/**
	 * Get the configuration payload identifier (IKEv1 only).
	 *
	 * @return			identifier
	 */
	uint16_t (*get_identifier) (cp_payload_t *this);

	/**
	 * Destroys an cp_payload_t object.
	 */
	void (*destroy) (cp_payload_t *this);
};

/**
 * Creates an empty configuration payload
 *
 * @param type		payload type, PLV2_CONFIGURATION or PLV1_CONFIGURATION
 * @return			empty configuration payload
 */
cp_payload_t *cp_payload_create(payload_type_t type);

/**
 * Creates an cp_payload_t with type and value
 *
 * @param type		payload type, PLV2_CONFIGURATION or PLV1_CONFIGURATION
 * @param cfg_type	type of configuration payload to create
 * @return			created configuration payload
 */
cp_payload_t *cp_payload_create_type(payload_type_t type, config_type_t cfg_type);

#endif /** CP_PAYLOAD_H_ @}*/

/*
 * Copyright (C) 2015 Andreas Steffen
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
 * @defgroup pb_mutual_capability_msg pb_mutual_capability_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_MUTUAL_CAPABILITY_MSG_H_
#define PB_MUTUAL_CAPABILITY_MSG_H_

typedef enum pb_tnc_mutual_protocol_type_t pb_tnc_mutual_protocol_type_t;
typedef struct pb_mutual_capability_msg_t pb_mutual_capability_msg_t;

#include "messages/pb_tnc_msg.h"

/**
 * PB-TNC mutual protocol types
 */
enum pb_tnc_mutual_protocol_type_t {
	PB_MUTUAL_HALF_DUPLEX =	(1 << 31),
	PB_MUTUAL_FULL_DUPLEX =	(1 << 30)
};

/**
 * enum name for pb_mutual_protocol_type_t.
 */
extern enum_name_t *pb_tnc_mutual_protocol_type_names;

/**
 * Class representing the PB-Mutual-Capabilities message type.
 */
struct pb_mutual_capability_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get the PB-TNC mutual protocol types
	 *
	 * @return				PB-TNC mutual protocol types
	 */
	uint32_t(*get_protocols)(pb_mutual_capability_msg_t *this);

};

/**
 * Create a PB-Mutual-Capability message
 *
 * @param protocols			Supported PB-TNC mutual protocols
 */
pb_tnc_msg_t* pb_mutual_capability_msg_create(uint32_t protocols);

/**
 * Create an unprocessed PB-Mutual-Capability message from raw data
 *
  * @param data		PB-Mutual-Capability message data
 */
pb_tnc_msg_t* pb_mutual_capability_msg_create_from_data(chunk_t data);

#endif /** PB_MUTUAL_CAPABILITY_MSG_ @}*/

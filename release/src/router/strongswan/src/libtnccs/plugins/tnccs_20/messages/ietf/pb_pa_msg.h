/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup pb_pa_msg pb_pa_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_PA_MSG_H_
#define PB_PA_MSG_H_

typedef struct pb_pa_msg_t pb_pa_msg_t;

#include "messages/pb_tnc_msg.h"

#include <pen/pen.h>

#define PB_PA_MSG_HEADER_SIZE	12

/**
 * Class representing the PB-PA message type.
 */
struct pb_pa_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get PA Message Vendor ID and Subtype
	 *
	 * @return				Vendor-specific PA Subtype
	 */
	pen_type_t (*get_subtype)(pb_pa_msg_t *this);

	/**
	 * Get Posture Collector ID
	 *
	 * @return				Posture Collector ID
	 */
	uint16_t (*get_collector_id)(pb_pa_msg_t *this);

	/**
	 * Get Posture Validator ID
	 *
	 * @return				Posture Validator ID
	 */
	uint16_t (*get_validator_id)(pb_pa_msg_t *this);

	/**
	 * Get the PA Message Body
	 *
	 * @return				PA Message Body
	 */
	chunk_t (*get_body)(pb_pa_msg_t *this);

	/**
	 * Get the exclusive flag
	 *
	 * @return				exclusive flag
	 */
	bool (*get_exclusive_flag)(pb_pa_msg_t *this);

};

/**
 * Create a PB-PA message from parameters
 *
 * @param vendor_id			PA Message Vendor ID
 * @param subtype			PA Subtype		
 * @param collector_id		Posture Collector ID
 * @param validator_id		Posture Validator ID
 * @param excl				Exclusive Flag
 * @param msg_body		 	PA Message Body
 */
pb_tnc_msg_t *pb_pa_msg_create(uint32_t vendor_id, uint32_t subtype,
							   uint16_t collector_id, uint16_t validator_id,
							   bool excl, chunk_t msg_body);

/**
 * Create an unprocessed PB-PA message from raw data
 *
  * @param data		PB-PA message data
 */
pb_tnc_msg_t* pb_pa_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/

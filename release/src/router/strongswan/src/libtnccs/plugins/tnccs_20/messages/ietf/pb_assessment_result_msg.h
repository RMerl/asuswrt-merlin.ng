/*
 * Copyright (C) 2010 Sansar Choinyambuu
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
 * @defgroup pb_assessment_result_msg pb_assessment_result_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_ASSESSMENT_RESULT_MSG_H_
#define PB_ASSESSMENT_RESULT_MSG_H_

typedef struct pb_assessment_result_msg_t pb_assessment_result_msg_t;

#include "messages/pb_tnc_msg.h"

/**
 * Class representing the PB-Assessment-Result message type.
 */
struct pb_assessment_result_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;

	/**
	 * Get PB Assessment result
	 *
	 * @return			PB Assessment result
	 */
	uint32_t (*get_assessment_result)(pb_assessment_result_msg_t *this);
};

/**
 * Create a PB-Assessment-Result message from parameters
 *
 * @param assessment_result		Assessment result code
 */
pb_tnc_msg_t* pb_assessment_result_msg_create(uint32_t assessment_result);

/**
 * Create an unprocessed PB-Assessment-Result message from raw data
 *
  * @param data		PB-Assessment-Result message data
 */
pb_tnc_msg_t* pb_assessment_result_msg_create_from_data(chunk_t data);

#endif /** PB_PA_MSG_H_ @}*/

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
 * @defgroup pb_experimental_msg pb_experimental_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_EXPERIMENTAL_MSG_H_
#define PB_EXPERIMENTAL_MSG_H_

typedef struct pb_experimental_msg_t pb_experimental_msg_t;

#include "messages/pb_tnc_msg.h"

/**
 * Class representing the PB-Experimental message type.
 */
struct pb_experimental_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;
};

/**
 * Create a PB-Experimental message from parameters
 *
 * @param body			message body
 */
pb_tnc_msg_t* pb_experimental_msg_create(chunk_t body);

/**
 * Create an unprocessed PB-Experimental message from raw data
 *
 * @param data			PB-Experimental message data
 */
pb_tnc_msg_t* pb_experimental_msg_create_from_data(chunk_t data);

#endif /** PB_EXPERIMENTAL_MSG_H_ @}*/

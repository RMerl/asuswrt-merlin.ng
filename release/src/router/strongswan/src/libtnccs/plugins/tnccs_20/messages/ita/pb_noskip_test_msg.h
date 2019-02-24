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
 * @defgroup pb_noskip_test_msg pb_noskip_test_msg
 * @{ @ingroup tnccs_20
 */

#ifndef PB_NOSKIP_TEST_MSG_H_
#define PB_NOSKIP_TEST_MSG_H_

typedef struct pb_noskip_test_msg_t pb_noskip_test_msg_t;

#include "messages/pb_tnc_msg.h"

/**
 * Class representing the PB-Noskip-Test message type.
 */
struct pb_noskip_test_msg_t {

	/**
	 * PB-TNC Message interface
	 */
	pb_tnc_msg_t pb_interface;
};

/**
 * Create a PB-Noskip-Test message from parameters
 */
pb_tnc_msg_t* pb_noskip_test_msg_create(void);

#endif /** PB_NOSKIP_TEST_MSG_H_ @}*/

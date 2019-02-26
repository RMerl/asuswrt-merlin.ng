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
 * @defgroup imc_imv_msg imc_imv_msg
 * @{ @ingroup tnccs_11
 */

#ifndef IMC_IMV_MSG_H_
#define IMC_IMV_MSG_H_

typedef struct imc_imv_msg_t imc_imv_msg_t;

#include "tnccs_msg.h"

#include <tncif.h>

/**
 * Class representing the PB-PA message type.
 */
struct imc_imv_msg_t {

	/**
	 * TNCCS Message interface
	 */
	tnccs_msg_t tnccs_msg_interface;

	/**
	 * Get IMC-IMV message type
	 *
	 * @return				IMC-IMV message type
	 */
	TNC_MessageType (*get_msg_type)(imc_imv_msg_t *this);

	/**
	 * Get IMC-IMV message body
	 *
	 * @return				IMC-IMV message body
	 */
	chunk_t (*get_msg_body)(imc_imv_msg_t *this);
};

/**
 * Create an IMC-IMV message from XML-encoded message node
 *
 * @param node				XML-encoded message node
 * @param errors			linked list of TNCCS error messages
*/
tnccs_msg_t *imc_imv_msg_create_from_node(xmlNodePtr node, linked_list_t *errors);

/**
 * Create an IMC-IMV message from parameters
 *
 * @param msg_type			IMC-IMV message type
 * @param msg_body			IMC-IMV message body
 */
tnccs_msg_t *imc_imv_msg_create(TNC_MessageType msg_type, chunk_t msg_body);

#endif /** IMC_IMV_MSG_H_ @}*/

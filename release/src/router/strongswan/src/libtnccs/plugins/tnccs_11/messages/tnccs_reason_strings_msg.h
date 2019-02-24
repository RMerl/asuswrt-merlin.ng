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
 * @defgroup tnccs_reason_strings_msg tnccs_reason_strings_msg
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_REASON_STRINGS_MSG_H_
#define TNCCS_REASON_STRINGS_MSG_H_

typedef struct tnccs_reason_strings_msg_t tnccs_reason_strings_msg_t;

#include "tnccs_msg.h"

/**
 * Class representing the TNCCS-ReasonStrings message type
 */
struct tnccs_reason_strings_msg_t {

	/**
	 * TNCCS Message interface
	 */
	tnccs_msg_t tnccs_msg_interface;

	/**
	 * Get reason string and language
	 *
	 * @param language		reason language
	 * @return				reason string
	 */
	chunk_t (*get_reason)(tnccs_reason_strings_msg_t *this, chunk_t *language);
};

/**
 * Create a TNCCS-ReasonStrings message from XML-encoded message node
 *
 * @param node				XML-encoded message node
 * @param errors			linked list of TNCCS error messages
 */
tnccs_msg_t *tnccs_reason_strings_msg_create_from_node(xmlNodePtr node,
													   linked_list_t *errors);

/**
 * Create a TNCCS-ReasonStrings message from parameters
 *
 * @param reason			reason string
 * @param language			reason language
 */
tnccs_msg_t *tnccs_reason_strings_msg_create(chunk_t reason, chunk_t language);

#endif /** TNCCS_REASON_STRINGS_MSG_H_ @}*/

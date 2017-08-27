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
 * @defgroup tnccs_error_msg tnccs_error_msg
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_ERROR_MSG_H_
#define TNCCS_ERROR_MSG_H_

typedef enum tnccs_error_type_t tnccs_error_type_t;
typedef struct tnccs_error_msg_t tnccs_error_msg_t;

#include "tnccs_msg.h"

/**
 * TNCCS error types as defined in section 8.1.4 of TCG TNC IF-TNCCS v1.2
 */
enum tnccs_error_type_t {
	TNCCS_ERROR_BATCH_TOO_LONG,
	TNCCS_ERROR_MALFORMED_BATCH,
	TNCCS_ERROR_INVALID_BATCH_ID,
	TNCCS_ERROR_INVALID_RECIPIENT_TYPE,
	TNCCS_ERROR_INTERNAL_ERROR,
	TNCCS_ERROR_OTHER
};

/**
 * enum name for tnccs_error_type_t.
 */
extern enum_name_t *tnccs_error_type_names;

/**
 * Class representing the TNCCS-Error message type
 */
struct tnccs_error_msg_t {

	/**
	 * TNCCS Message interface
	 */
	tnccs_msg_t tnccs_msg_interface;

	/**
	 * Get error message and type
	 *
	 * @param type		TNCCS error type
	 * @return			arbitrary error message
	 */
	char* (*get_message)(tnccs_error_msg_t *this, tnccs_error_type_t *type);
};

/**
 * Create a TNCCS-Error message from XML-encoded message node
 *
 * @param node			XML-encoded message node
 */
tnccs_msg_t *tnccs_error_msg_create_from_node(xmlNodePtr node);

/**
 * Create a TNCCS-Error message from parameters
 *
 * @param type			TNCCS error type
 * @param msg			arbitrary error message
 */
tnccs_msg_t *tnccs_error_msg_create(tnccs_error_type_t type, char *msg);

#endif /** TNCCS_ERROR_MSG_H_ @}*/

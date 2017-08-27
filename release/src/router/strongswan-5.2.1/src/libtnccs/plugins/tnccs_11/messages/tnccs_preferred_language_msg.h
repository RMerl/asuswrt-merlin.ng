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
 * @defgroup tnccs_preferred_language_msg tnccs_preferred_language_msg
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_PREFERRED_LANGUAGE_MSG_H_
#define TNCCS_PREFERRED_LANGUAGE_MSG_H_

typedef struct tnccs_preferred_language_msg_t tnccs_preferred_language_msg_t;

#include "tnccs_msg.h"

#include <tncif.h>

/**
 * Class representing the TNCCS-PreferredLanguage message type
 */
struct tnccs_preferred_language_msg_t {

	/**
	 * TNCCS Message interface
	 */
	tnccs_msg_t tnccs_msg_interface;

	/**
	 * Get preferred language string
	 *
	 * @return				preferred language string
	 */
	char* (*get_preferred_language)(tnccs_preferred_language_msg_t *this);
};

/**
 * Create a TNCCS-PreferredLanguage message from XML-encoded message node
 *
 * @param node				XML-encoded message node
 * @param errors			linked list of TNCCS error messages
 */
tnccs_msg_t *tnccs_preferred_language_msg_create_from_node(xmlNodePtr node,
													linked_list_t *errors);

/**
 * Create a TNCCS-PreferredLanguage message from parameters
 *
 * @param language			preferred language string
 */
tnccs_msg_t *tnccs_preferred_language_msg_create(char *language);

#endif /** TNCCS_PREFERRED_LANGUAGE_MSG_H_ @}*/

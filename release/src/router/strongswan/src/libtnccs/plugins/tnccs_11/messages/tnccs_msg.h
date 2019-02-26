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
 * @defgroup tnccs_msg tnccs_msg
 * @{ @ingroup tnccs_11
 */

#ifndef TNCCS_MSG_H_
#define TNCCS_MSG_H_

typedef enum tnccs_msg_type_t tnccs_msg_type_t;
typedef struct tnccs_msg_t tnccs_msg_t;

#include <library.h>
#include <collections/linked_list.h>
#include <libxml/parser.h>

/**
 * TNCC-TNCS messages as defined in section 2.8.5 of TCG TNC IF-TNCCS v1.2
 */
enum tnccs_msg_type_t {
	IMC_IMV_MSG =						0,
	TNCCS_MSG_RECOMMENDATION =			1,
	TNCCS_MSG_ERROR =					2,
	TNCCS_MSG_PREFERRED_LANGUAGE =		3,
	TNCCS_MSG_REASON_STRINGS =			4,
	TNCCS_MSG_TNCS_CONTACT_INFO =		5,
	TNCCS_MSG_ROOF =					5
};

/**
 * enum name for tnccs_msg_type_t.
 */
extern enum_name_t *tnccs_msg_type_names;

/**
 * Generic interface for all TNCCS message types.
 *
 * To handle all messages in a generic way, this interface
 * must be implemented by each message type.
 */
struct tnccs_msg_t {

	/**
	 * Get the TNCCS Message Type
	 *
	 * @return				TNCCS Message Type
	 */
	tnccs_msg_type_t (*get_type)(tnccs_msg_t *this);

	/**
	 * Get the XML-encoded Message Node
	 *
	 * @return				Message Node
	 */
	xmlNodePtr (*get_node)(tnccs_msg_t *this);

	/**
	 * Process the TNCCS Message
	 *
	 * @return				return processing status
	 */
	status_t (*process)(tnccs_msg_t *this);

	/**
	 * Get a new reference to the message.
	 *
	 * @return			this, with an increased refcount
	 */
	tnccs_msg_t* (*get_ref)(tnccs_msg_t *this);

	/**
	 * Destroys a tnccs_msg_t object.
	 */
	void (*destroy)(tnccs_msg_t *this);
};

/**
 * Create a pre-processed TNCCS message
 *
 * Useful for the parser which wants a generic constructor for all
 * tnccs_msg_t types.
 *
 * @param node		TNCCS message node
 * @param errors	linked list of TNCCS error messages
 */
tnccs_msg_t* tnccs_msg_create_from_node(xmlNodePtr node, linked_list_t *errors);

#endif /** TNCCS_MSG_H_ @}*/

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

#include "tnccs_tncs_contact_info_msg.h"

#include <utils/debug.h>

typedef struct private_tnccs_tncs_contact_info_msg_t private_tnccs_tncs_contact_info_msg_t;

/**
 * Private data of a tnccs_tncs_contact_info_msg_t object.
 *
 */
struct private_tnccs_tncs_contact_info_msg_t {
	/**
	 * Public tnccs_tncs_contact_info_msg_t interface.
	 */
	tnccs_tncs_contact_info_msg_t public;

	/**
	 * TNCCS message type
	 */
	tnccs_msg_type_t type;

	/**
	 * XML-encoded message node
	 */
	xmlNodePtr node;
};

METHOD(tnccs_msg_t, get_type, tnccs_msg_type_t,
	private_tnccs_tncs_contact_info_msg_t *this)
{
	return this->type;
}

METHOD(tnccs_msg_t, get_node, xmlNodePtr,
	private_tnccs_tncs_contact_info_msg_t *this)
{
	return this->node;
}

METHOD(tnccs_msg_t, destroy, void,
	private_tnccs_tncs_contact_info_msg_t *this)
{
	free(this);
}

/**
 * See header
 */
tnccs_msg_t *tnccs_tncs_contact_info_msg_create_from_node(xmlNodePtr node,
													linked_list_t *errors)
{
	private_tnccs_tncs_contact_info_msg_t *this;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
		},
		.type = TNCCS_MSG_TNCS_CONTACT_INFO,
		.node = node,
	);

	return &this->public.tnccs_msg_interface;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_tncs_contact_info_msg_create(void)
{
	private_tnccs_tncs_contact_info_msg_t *this;
	xmlNodePtr n /*, n2 */;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
		},
		.type = TNCCS_MSG_TNCS_CONTACT_INFO,
		.node =  xmlNewNode(NULL, "TNCC-TNCS-Message"),
	);

	/* add the message type number in hex */
	n = xmlNewNode(NULL, "Type");
	xmlNodeSetContent(n, "00000005");
	xmlAddChild(this->node, n);

	n = xmlNewNode(NULL, "XML");
	xmlAddChild(this->node, n);

/* TODO
	n2 = xmlNewNode(NULL, enum_to_name(tnccs_msg_type_names, this->type));
	xmlNodeSetContent(n2, language);
	xmlAddChild(n, n2);
*/

	return &this->public.tnccs_msg_interface;
}

/*
 * Copyright (C) 2010 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2006 Mike McCauley (mikem@open.com.au)
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

#include "tnccs_error_msg.h"

#include <utils/debug.h>

ENUM(tnccs_error_type_names, TNCCS_ERROR_BATCH_TOO_LONG, TNCCS_ERROR_OTHER,
	"batch-too-long",
	"malformed-batch",
	"invalid-batch-id",
	"invalid-recipient-type",
	"internal-error",
	"other"
);

typedef struct private_tnccs_error_msg_t private_tnccs_error_msg_t;

/**
 * Private data of a tnccs_error_msg_t object.
 *
 */
struct private_tnccs_error_msg_t {
	/**
	 * Public tnccs_error_msg_t interface.
	 */
	tnccs_error_msg_t public;

	/**
	 * TNCCS message type
	 */
	tnccs_msg_type_t type;

	/**
	 * XML-encoded message node
	 */
	xmlNodePtr node;

	/**
	 * Error type
	 */
	tnccs_error_type_t error_type;

	/**
	 * Error message
	 */
	char *error_msg;

	/**
	 * reference count
	 */
	refcount_t ref;
};

METHOD(tnccs_msg_t, get_type, tnccs_msg_type_t,
	private_tnccs_error_msg_t *this)
{
	return this->type;
}

METHOD(tnccs_msg_t, get_node, xmlNodePtr,
	private_tnccs_error_msg_t *this)
{
	return this->node;
}

METHOD(tnccs_msg_t, get_ref, tnccs_msg_t*,
	private_tnccs_error_msg_t *this)
{
	ref_get(&this->ref);
	return &this->public.tnccs_msg_interface;
}

METHOD(tnccs_msg_t, destroy, void,
	private_tnccs_error_msg_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->error_msg);
		free(this);
	}
}

METHOD(tnccs_error_msg_t, get_message, char*,
	private_tnccs_error_msg_t *this, tnccs_error_type_t *type)
{
	*type = this->error_type;

	return this->error_msg;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_error_msg_create_from_node(xmlNodePtr node)
{
	private_tnccs_error_msg_t *this;
	xmlChar *error_type_name, *error_msg;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_message = _get_message,
		},
		.type = TNCCS_MSG_ERROR,
		.ref = 1,
		.node = node,
		.error_type = TNCCS_ERROR_OTHER,
	);

	error_type_name = xmlGetProp(node, "type");
	if (error_type_name)
	{
		if (!enum_from_name(tnccs_error_type_names, error_type_name,
							&this->error_type))
		{
			this->error_type = TNCCS_ERROR_OTHER;
		}
		xmlFree(error_type_name);
	}

	error_msg = xmlNodeGetContent(node);
	if (error_msg)
	{
		this->error_msg = strdup(error_msg);
		xmlFree(error_msg);
	}

	return &this->public.tnccs_msg_interface;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_error_msg_create(tnccs_error_type_t type, char *msg)
{
	private_tnccs_error_msg_t *this;
	xmlNodePtr n, n2;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_message = _get_message,
		},
		.type = TNCCS_MSG_ERROR,
		.ref = 1,
		.node =  xmlNewNode(NULL, "TNCC-TNCS-Message"),
		.error_type = type,
		.error_msg  = strdup(msg),
	);

	DBG1(DBG_TNC, "%s", msg);

	n = xmlNewNode(NULL, "Type");
	xmlNodeSetContent(n, "00000002");
	xmlAddChild(this->node, n);

	n = xmlNewNode(NULL, "XML");
	xmlAddChild(this->node, n);

	n2 = xmlNewNode(NULL, enum_to_name(tnccs_msg_type_names, this->type));
	xmlNewProp(n2, "type", enum_to_name(tnccs_error_type_names, type));
	xmlNodeSetContent(n2, msg);
	xmlAddChild(n, n2);

	return &this->public.tnccs_msg_interface;
}

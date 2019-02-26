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

#include "tnccs_reason_strings_msg.h"
#include "tnccs_error_msg.h"

#include <utils/debug.h>

typedef struct private_tnccs_reason_strings_msg_t private_tnccs_reason_strings_msg_t;

/**
 * Private data of a tnccs_reason_strings_msg_t object.
 *
 */
struct private_tnccs_reason_strings_msg_t {
	/**
	 * Public tnccs_reason_strings_msg_t interface.
	 */
	tnccs_reason_strings_msg_t public;

	/**
	 * TNCCS message type
	 */
	tnccs_msg_type_t type;

	/**
	 * XML-encoded message node
	 */
	xmlNodePtr node;

	/**
	 * Reason String
	 */
	chunk_t reason;

	/**
	 * Reason Language
	 */
	chunk_t language;
};

METHOD(tnccs_msg_t, get_type, tnccs_msg_type_t,
	private_tnccs_reason_strings_msg_t *this)
{
	return this->type;
}

METHOD(tnccs_msg_t, get_node, xmlNodePtr,
	private_tnccs_reason_strings_msg_t *this)
{
	return this->node;
}

METHOD(tnccs_msg_t, destroy, void,
	private_tnccs_reason_strings_msg_t *this)
{
	free(this->reason.ptr);
	free(this->language.ptr);
	free(this);
}

METHOD(tnccs_reason_strings_msg_t, get_reason, chunk_t,
	private_tnccs_reason_strings_msg_t *this, chunk_t *language)
{
	*language = this->language;

	return this->reason;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_reason_strings_msg_create_from_node(xmlNodePtr node,
													   linked_list_t *errors)
{
	private_tnccs_reason_strings_msg_t *this;
	char *error_msg, *lang_string, *reason_string;
	tnccs_error_type_t error_type = TNCCS_ERROR_MALFORMED_BATCH;
	tnccs_msg_t *msg;
	xmlNodePtr child;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_reason = _get_reason,
		},
		.type = TNCCS_MSG_REASON_STRINGS,
		.node = node,
	);

	if (xmlStrcmp(node->name, "TNCCS-ReasonStrings"))
	{
		error_msg = "TNCCS-ReasonStrings tag expected";
		goto fatal;
	}

	child = node->xmlChildrenNode;
	while (child)
	{
		if (xmlIsBlankNode(child))
		{
			child = child->next;
			continue;
		}
		if (xmlStrcmp(child->name, "ReasonString"))
		{
			error_msg = "ReasonString tag expected";
			goto fatal;
		}
		break;
	}

	lang_string = xmlGetProp(child, "lang");
	if (!lang_string)
	{
		lang_string = strdup("");
	}
	this->language = chunk_clone(chunk_from_str(lang_string));
	xmlFree(lang_string);

	reason_string = xmlNodeGetContent(child);
	this->reason = chunk_clone(chunk_from_str(reason_string));
	xmlFree(reason_string);

	return &this->public.tnccs_msg_interface;

fatal:
	msg = tnccs_error_msg_create(error_type, error_msg);
	errors->insert_last(errors, msg);
	destroy(this);
	return NULL;
}

/**
 * See header
 */
tnccs_msg_t *tnccs_reason_strings_msg_create(chunk_t reason, chunk_t language)
{
	private_tnccs_reason_strings_msg_t *this;
	xmlNodePtr n, n2, n3;

	INIT(this,
		.public = {
			.tnccs_msg_interface = {
				.get_type = _get_type,
				.get_node = _get_node,
				.destroy = _destroy,
			},
			.get_reason = _get_reason,
		},
		.type = TNCCS_MSG_REASON_STRINGS,
		.node =  xmlNewNode(NULL, "TNCC-TNCS-Message"),
		.reason = chunk_create_clone(malloc(reason.len + 1), reason),
		.language = chunk_create_clone(malloc(language.len + 1), language),
	);

	/* add NULL termination for XML string representation */
	this->reason.ptr[this->reason.len] = '\0';
	this->language.ptr[this->language.len] = '\0';

	/* add the message type number in hex */
	n = xmlNewNode(NULL, "Type");
	xmlNodeSetContent(n, "00000004");
	xmlAddChild(this->node, n);

	n = xmlNewNode(NULL, "XML");
	xmlAddChild(this->node, n);

	n2 = xmlNewNode(NULL, enum_to_name(tnccs_msg_type_names, this->type));

	/* could add multiple reasons here, if we had them */

	n3 = xmlNewNode(NULL, "ReasonString");
	xmlNewProp(n3, "xml:lang", this->language.ptr);
	xmlNodeSetContent(n3, this->reason.ptr);
	xmlAddChild(n2, n3);
	xmlAddChild(n, n2);

	return &this->public.tnccs_msg_interface;
}
